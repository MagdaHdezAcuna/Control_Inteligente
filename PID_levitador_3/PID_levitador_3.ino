// ================= Levitador: mapeo volt -> cm por tabla (interpolacion) ================
// Tabla de calibración: (cm, voltaje) que indicaste
// 0 -> 0.475
// 5 -> 0.533
// 10 -> 0.61
// 15 -> 0.727
// 20 -> 0.859
// 25 -> 0.971
// 30 -> 1.164
// 35 -> 1.467
// 40 -> 1.951
// 45 -> 2.97
// ======================================================================================

const int PIN_SHARP = A1;    // pin del sensor Sharp
const int PIN_PWM   = 9;     // pin PWM de salida

// muestreo y filtros
const unsigned long DT_US = 20000; // 20 ms period
const float DT = DT_US * 1e-6;
const int   N_SAMPLES = 8;         // promediado de ADC (reduce ruido)
const float ALPHA = 0.25;         // filtro exponencial para altura (0..1)

// rango
const float H_MIN = 0.0;
const float H_MAX = 45.0; // ajustado a 45 porque la tabla llega a 45cm

// PID (valores iniciales, ajústalos después)
float kp = 0.075;
float ki = 0.01;
float kd = 0.03;

float setpoint = 10.0; // cm

// tabla calibración (ordenada por voltaje ascendente)
const int TABLE_N = 10;
const float table_cm[TABLE_N]    = { 0.0,  5.0, 10.0, 15.0, 20.0, 25.0, 30.0, 35.0, 40.0, 45.0 };
const float table_volt[TABLE_N]  = {0.535,0.583,0.610,0.727,0.859,0.971,1.164,1.467,1.951,2.970 };

// invertir la relación si el sensor está orientado al revés (true = invertir)
const bool INVERT_MAPPING = false;

// variables PID / control
float altura = 0.0;        // altura filtrada usada por el controlador
float err = 0.0, err_prev = 0.0, integ = 0.0, deriv = 0.0, u = 0.0;

unsigned long t1, t2;
const bool PRINT_VERBOSE = true;

// ----------------- utilidades -----------------
float adcToVolt(int raw){
  return raw * (5.0f / 1023.0f);
}

// promedia N muestras ADC (lee con pequeño delay interno)
int readAdcAvg(int pin, int n){
  long s = 0;
  for(int i=0;i<n;i++){
    s += analogRead(pin);
    delay(2); // pequeño retardo para estabilizar lectura
  }
  return (int)(s / n);
}

// interpolacion lineal volt -> cm usando la tabla
float voltToCm(float v){
  // si fuera necesario invertir la relación:
  if (INVERT_MAPPING) {
    // si invertimos, mapeamos v proporcionalmente al rango invertido:
    // Primero calculamos cm normal y luego lo invertimos respecto al rango de la tabla.
    // (alternativa: invertir los arrays)
    // Para simplicidad: se calculará el cm normal y luego cm_final = max - (cm - min)
  }

  // fuera de la tabla: extrapolar o limitar
  if (v <= table_volt[0]) return (INVERT_MAPPING ? table_cm[TABLE_N-1] : table_cm[0]);
  if (v >= table_volt[TABLE_N-1]) return (INVERT_MAPPING ? table_cm[0] : table_cm[TABLE_N-1]);

  // buscar intervalo
  for(int i=0;i<TABLE_N-1;i++){
    float v1 = table_volt[i];
    float v2 = table_volt[i+1];
    if (v >= v1 && v <= v2){
      float d1 = table_cm[i];
      float d2 = table_cm[i+1];
      // interpolacion lineal
      float t = (v - v1) / (v2 - v1);
      float d = d1 + t * (d2 - d1);
      if (INVERT_MAPPING){
        // invertir respecto al rango completo de la tabla
        float d_min = table_cm[0];
        float d_max = table_cm[TABLE_N-1];
        d = d_max - (d - d_min);
      }
      return constrain(d, H_MIN, H_MAX);
    }
  }
  // seguridad (no deberia llegar aqui)
  return (INVERT_MAPPING ? table_cm[0] : table_cm[TABLE_N-1]);
}

// ----------------- setup -----------------
void setup(){
  Serial.begin(115200);
  pinMode(PIN_PWM, OUTPUT);
  analogWrite(PIN_PWM, 0);
  Serial.println(F("LEVITADOR - mapeo por tabla (volt->cm)"));
  Serial.println(F("Comandos serial: escribir numero para setpoint (ej: 20) o 'PID kp ki kd'"));
  Serial.println();
}

// procesa comandos serial (setpoint o PID)
void processSerial(){
  if (Serial.available()==0) return;
  String s = Serial.readStringUntil('\n'); s.trim();
  if (s.length()==0) return;
  if (s.startsWith("PID") || s.startsWith("pid")){
    float k1=kp,k2=ki,k3=kd;
    int n = sscanf(s.c_str(), "PID %f %f %f", &k1, &k2, &k3);
    if (n>=1) kp=k1; if (n>=2) ki=k2; if (n>=3) kd=k3;
    Serial.print("PID set: kp="); Serial.print(kp); Serial.print(" ki="); Serial.print(ki); Serial.print(" kd="); Serial.println(kd);
    return;
  }
  float v = s.toFloat();
  if (!(v==0.0 && s!="0" && s!="0.0")){
    if (v>=H_MIN && v<=H_MAX){ setpoint = v; Serial.print("Setpoint="); Serial.println(setpoint);}
    else Serial.println("Setpoint fuera de rango");
  }
}

// ----------------- loop -----------------
void loop(){
  t1 = micros();
  processSerial();

  // 1) leer ADC promediado y convertir a volt
  int raw = readAdcAvg(PIN_SHARP, N_SAMPLES);
  float volt = adcToVolt(raw);

  // 2) convertir volt -> distancia usando tabla (interpolacion)
  float measured = voltToCm(volt);

  // 3) filtrar la altura (exponencial) para estabilizar lecturas
  static float altura_f = 0.0;
  altura_f = ALPHA * measured + (1.0 - ALPHA) * altura_f;
  altura = altura_f;

  // 4) PID (usando altura filtrada)
  err = setpoint - altura;
  integ += err * DT;
  integ = constrain(integ, -100.0, 100.0);
  deriv = (err - err_prev) / DT;
  // salida PID en "voltios virtuales" (0..5V)
  u = kp * err + ki * integ + kd * deriv;
  err_prev = err;

  float vOut = constrain(u, 0.0, 5.0);             // limitar a 0-5V de control
  int pwm = (int)round((vOut / 5.0) * 255.0);     // mapear 0-5V -> 0-255 PWM
  pwm = constrain(pwm, 0, 255);
  analogWrite(PIN_PWM, pwm);

  // Telemetría: raw,volt,measured_cm,altura_filtrada,setpoint,vOut,pwm
  if (PRINT_VERBOSE){
    Serial.print(raw); Serial.print(',');
    Serial.print(volt,4); Serial.print(',');
    Serial.print(measured,3); Serial.print(',');   // distancia sin filtrar (de tabla)
    Serial.print(altura,3); Serial.print(',');
    Serial.print(setpoint,3); Serial.print(',');
    Serial.print(vOut,3); Serial.print(',');
    Serial.println(pwm);
  }

  // periodo fijo
  t2 = micros();
  while((t2 - t1) < DT_US) t2 = micros();
}
