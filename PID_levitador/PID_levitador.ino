// Levitador neumático - calibración aplicada (modelo inverso d = A/(V - B))
// A y B calculados desde tus mediciones

// ----- PINES -----
const int PIN_SHARP = A0;
const int PIN_PWM   = 9;   // salida PWM (0-5V equivalente en duty cycle)

// ----- TIEMPOS -----
const unsigned long DT_US = 20000; // 20 ms (50 Hz)
const float DT = DT_US * 1e-6;

// ----- RANGO -----
const float H_MIN = 0.0;   // cm
const float H_MAX = 40.0;  // cm

// ----- MODELO SHARP (resultado de la calibración) -----
// d = A / (V - B)
const float CAL_A = 16.049792987278796;
const float CAL_B = 0.45659888720364566;

// ----- PID (valores iniciales, AJUSTAR) -----
float kp = 0.08;
float ki = 0.02;
float kd = 0.01;

float setpoint = 10.0; // cm por defecto

// ----- VARIABLES -----
float altura = 0.0;
float err = 0.0, err_prev = 0.0, integ = 0.0, deriv = 0.0, u = 0.0;

unsigned long t1, t2;

// ----- VARIABLES DE DEBUG -----
const bool PRINT_VERBOSE = true; // si quieres menos salida, pon false

// ----- FUNCIONES AUX -----
float adcToVolt(int raw) {
  return raw * (5.0 / 1023.0);
}

// Modelo calibrado (inverso)
float sharpVoltToCm(float v) {
  float denom = v - CAL_B;
  if (denom <= 0.02) return H_MAX; // fuera de rango -> distancia máxima
  float d = CAL_A / denom;
  return constrain(d, H_MIN, H_MAX);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_PWM, OUTPUT);
  analogWrite(PIN_PWM, 0);
  Serial.println(F("LEVITADOR: inicio con calibracion aplicada (salida 0-5V)"));
  Serial.println(F("Envía por Serial un número (ej: 20) para cambiar el setpoint"));
  Serial.println(F("O escribe: PID kp ki kd   (ej: PID 0.08 0.02 0.01)"));
  Serial.println();
}

// Procesa comandos simples por serial para cambiar setpoint o PID
void processSerial() {
  if (Serial.available() == 0) return;
  String s = Serial.readStringUntil('\n');
  s.trim();
  if (s.length() == 0) return;

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

  // si solo número -> setpoint
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

void loop() {
  t1 = micros();
  processSerial();

  // --- Lectura sensor ---
  int raw = analogRead(PIN_SHARP);
  float volt = adcToVolt(raw);
  altura = sharpVoltToCm(volt);

  // --- PID ---
  err = setpoint - altura;
  integ += err * DT;
  integ = constrain(integ, -100.0, 100.0); // anti-windup
  deriv = (err - err_prev) / DT;
  u = kp * err + ki * integ + kd * deriv; 
  err_prev = err;

  // --- salida directa a PWM 0-5V ---
  // El PID genera "u" en unidades arbitrarias -> mapear a PWM
  // Aquí asumimos u entre 0 y 5 V aprox -> PWM 0-255
  float vOut = constrain(u, 0.0, 5.0);
  int pwm = map(vOut * 100, 0, 500, 0, 255); // 0-5V → 0-255

  analogWrite(PIN_PWM, pwm);

  // --- Telemetría ---
  if (PRINT_VERBOSE) {
    Serial.print(raw); Serial.print(',');
    Serial.print(volt,4); Serial.print(',');
    Serial.print(altura,3); Serial.print(',');
    Serial.print(setpoint,3); Serial.print(',');
    Serial.print(vOut,3); Serial.print(',');
    Serial.println(pwm);
  }

  // mantener periodo
  t2 = micros();
  while ((t2 - t1) < DT_US) t2 = micros();
}
