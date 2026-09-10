// ===============================================================
// Levitador neumático - calibración polinómica (curva de Excel)
// La altura se calcula en función del voltaje de salida del sensor
// y = 0.0016x^4 - 0.0264x^3 + 0.1618x^2 - 0.3011x + 0.6495
// ===============================================================

// ----- PINES -----
const int PIN_SHARP = A1;   // entrada analógica del sensor Sharp (0-5V)
const int PIN_PWM   = 9;    // salida PWM al ventilador (control velocidad)

// ----- TIEMPOS -----
const unsigned long DT_US = 20000; // periodo del lazo de control = 20 ms (50 Hz)
const float DT = DT_US * 1e-6;     // lo mismo expresado en segundos

// ----- RANGO -----
const float H_MIN = 0.0;   // altura mínima en cm (0 cm)
const float H_MAX = 40.0;  // altura máxima en cm (40 cm)

// ----- PID (ganancias iniciales, AJUSTAR) -----
float kp = 0.105;   // proporcional
float ki = 0.03;   // integral
float kd = 0.05;   // derivativo

float setpoint = 10.0; // altura deseada por defecto (10 cm)

// ----- VARIABLES DEL CONTROL -----
float altura = 0.0;      // altura medida (cm)
float err = 0.0;         // error = setpoint - altura
float err_prev = 0.0;    // error anterior (para derivada)
float integ = 0.0;       // parte integral acumulada
float deriv = 0.0;       // parte derivativa
float u = 0.0;           // salida del PID (en "voltios virtuales")

unsigned long t1, t2;    // tiempos para mantener periodo fijo

// ----- DEBUG -----
const bool PRINT_VERBOSE = true; // si es true imprime muchos datos en Serial

// ===============================================================
// FUNCIONES AUXILIARES
// ===============================================================

// Convierte lectura ADC (0-1023) a voltaje real (0-5V)
float adcToVolt(int raw) {
  return raw * (5.0 / 1023.0);
}

// Convierte voltaje medido -> altura (cm)
// usando el polinomio que ajustaste en Excel
float sharpVoltToCm(float v) {
  float d = 2.011*pow(v,5) -22.384*pow(v,4) + 95.135*pow(v,3) - 193.21*pow(v,2) + 200.14*v - 61.691;
  return constrain(d, H_MIN, H_MAX);
}

// ===============================================================
// CONFIGURACIÓN INICIAL
// ===============================================================
void setup() {
  Serial.begin(115200);     // habilita puerto serial para depuración
  pinMode(PIN_PWM, OUTPUT); // define pin de salida PWM
  analogWrite(PIN_PWM, 0);  // arranca con PWM en 0 (ventilador apagado)

  Serial.println(F("LEVITADOR: inicio con calibracion polinomica (salida 0-5V)"));
  Serial.println(F("Envía por Serial un número (ej: 20) para cambiar el setpoint"));
  Serial.println(F("O escribe: PID kp ki kd   (ej: PID 0.08 0.02 0.01)"));
  Serial.println();
}

// ===============================================================
// LECTURA DE COMANDOS SERIAL (para ajustar setpoint o PID)
// ===============================================================
void processSerial() {
  if (Serial.available() == 0) return;  // no hay datos -> salir

  String s = Serial.readStringUntil('\n'); // lee línea completa
  s.trim(); // elimina espacios
  if (s.length() == 0) return;

  // Si el comando empieza con "PID", interpreta como ajuste de ganancias
  if (s.startsWith("PID") || s.startsWith("pid")) {
    float k1=kp,k2=ki,k3=kd;
    int n = sscanf(s.c_str(), "PID %f %f %f", &k1, &k2, &k3);
    if (n >= 1) kp = k1;
    if (n >= 2) ki = k2;
    if (n >= 3) kd = k3;

    Serial.print("PID actualizado: kp="); Serial.print(kp,5);
    Serial.print(" ki="); Serial.print(ki,6);
    Serial.print(" kd="); Serial.println(kd,6);
    return;
  }

  // Si es solo un número -> lo toma como nuevo setpoint
  float v = s.toFloat();
  if (!(v == 0.0 && s != "0" && s != "0.0")) {
    if (v >= H_MIN && v <= H_MAX) {
      setpoint = v;
      Serial.print("Setpoint = "); Serial.print(setpoint); Serial.println(" cm");
    } else {
      Serial.println("Setpoint fuera de rango (0-40)");
    }
  } else {
    Serial.println("Comando no entendido.");
  }
}

// ===============================================================
// BUCLE PRINCIPAL
// ===============================================================
void loop() {
  t1 = micros();  // guarda tiempo de inicio de ciclo
  processSerial(); // revisa si llegaron comandos por Serial

  // --- 1. Lectura del sensor ---
  //int raw = analogRead(PIN_SHARP);   // ADC 0..1023
  //float volt = adcToVolt(raw);       // voltaje 0..5V
  //altura = sharpVoltToCm(volt);      // convierte a cm usando polinomio
  int raw = analogRead(PIN_SHARP);
  float volt = adcToVolt(raw);       // voltaje 0..5V
  altura = sharpVoltToCm(volt); 

  // --- 2. Calculo del PID ---
  err = setpoint - (altura -5);      // error actual
  integ += err * DT;            // acumula parte integral
  integ = constrain(integ, -100.0, 100.0); // evita saturación (anti-windup)
  deriv = (err - err_prev) / DT; // derivada = delta error / tiempo
  u = kp * err + ki * integ + kd * deriv; // salida PID
  err_prev = err; // guarda error actual

  // --- 3. Salida a PWM ---
  float vOut = constrain(u, 0.0, 5.0);      // limita a rango válido 0-5V
  int pwm = map(vOut * 100, 0, 500, 0, 255); // escala 0-5V → 0-255 PWM
  analogWrite(PIN_PWM, pwm);                // aplica al ventilador

  // --- 4. Telemetría (para graficar en PC) ---
  if (PRINT_VERBOSE) {
    // imprime: ADC, Voltaje, Altura, Setpoint, salida PID, PW0
    Serial.print(raw); Serial.print(',');
    //Serial.print(volt,4); Serial.print(',');
    Serial.print(altura,3); Serial.print(',');
    Serial.print(setpoint,3); Serial.print(',');
    //Serial.print(vOut,3); Serial.print(',');

    Serial.println(pwm);
  }

  // --- 5. Mantener periodo fijo (20 ms) ---
  t2 = micros();
  while ((t2 - t1) < DT_US) t2 = micros();
}
