// ===============================================================
// Levitador neumático - VL53L0X con PID adaptativo (gain scheduling)
// Sensor montado arriba, midiendo hacia abajo.
// Altura real = H_SENSOR - distancia_medida
// ===============================================================

#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// ----- OBJETO SENSOR -----
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// ----- PINES -----
const int PIN_PWM = 9;  // salida PWM al ventilador

// ----- TIEMPOS -----
const unsigned long DT_US = 20000; // periodo del lazo = 20 ms (50 Hz)
const float DT = DT_US * 1e-6;     // segundos

// ----- RANGO -----
const float H_MIN = 0.0;   // altura mínima (cm)
const float H_MAX = 45.0;  // altura máxima (cm)

// ----- ALTURA DEL SENSOR -----
// Ajusta según tu montaje (distancia del sensor a la base)
const float H_SENSOR = 51.0; // cm

// ----- PID (ganancias base) -----
float kp = 0.105;
float ki = 0.03;
float kd = 0.05;

// ----- SETPOINT -----
float setpoint = 10.0;  // altura deseada (cm)

// ----- VARIABLES DEL CONTROL -----
float altura = 0.0;    // altura medida
float err = 0.0;
float err_prev = 0.0;
float integ = 0.0;
float deriv = 0.0;
float u = 0.0;

unsigned long t1, t2;

// ----- DEBUG -----
const bool PRINT_VERBOSE = true;

// ===============================================================
// CONFIGURACIÓN INICIAL
// ===============================================================
void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(PIN_PWM, OUTPUT);
  analogWrite(PIN_PWM, 0);

  Serial.println(F("LEVITADOR: VL53L0X + PID con reglas expertas (gain scheduling)"));
  Serial.println(F("Envía por Serial un número (ej: 20) para cambiar el setpoint"));
  Serial.println(F("O escribe: PID kp ki kd   (ej: PID 0.08 0.02 0.01)"));
  Serial.println();

  if (!lox.begin()) {
    Serial.println(F("ERROR: No se detectó el VL53L0X. Verifica conexión I2C."));
    while (1);
  }
  Serial.println(F("Sensor VL53L0X inicializado correctamente."));
}

// ===============================================================
// LECTURA DE COMANDOS SERIAL
// ===============================================================
void processSerial() {
  if (Serial.available() == 0) return;

  String s = Serial.readStringUntil('\n');
  s.trim();
  if (s.length() == 0) return;

  if (s.startsWith("PID") || s.startsWith("pid")) {
    float k1 = kp, k2 = ki, k3 = kd;
    int n = sscanf(s.c_str(), "PID %f %f %f", &k1, &k2, &k3);
    if (n >= 1) kp = k1;
    if (n >= 2) ki = k2;
    if (n >= 3) kd = k3;

    Serial.print("PID actualizado: kp="); Serial.print(kp,5);
    Serial.print(" ki="); Serial.print(ki,6);
    Serial.print(" kd="); Serial.println(kd,6);
    return;
  }

  float v = s.toFloat();
  if (!(v == 0.0 && s != "0" && s != "0.0")) {
    if (v >= H_MIN && v <= H_MAX) {
      setpoint = v;
      Serial.print("Setpoint = "); Serial.print(setpoint); Serial.println(" cm");
    } else {
      Serial.println("Setpoint fuera de rango (0-45)");
    }
  } else {
    Serial.println("Comando no entendido.");
  }
}

// ===============================================================
// BUCLE PRINCIPAL
// ===============================================================
void loop() {
  t1 = micros();
  processSerial();

  // --- 1. Lectura del sensor VL53L0X ---
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) { // 4 = fuera de rango
    float d_sensor = measure.RangeMilliMeter / 10.0; // mm → cm
    altura = H_SENSOR - d_sensor; // ajuste geométrico
    altura = constrain(altura, H_MIN, H_MAX);
  } else {
    Serial.println(F("Advertencia: lectura fuera de rango"));
  }

  // --- 2. Cálculo del PID base ---
  err = setpoint - altura;
  integ += err * DT;
  integ = constrain(integ, -100.0, 100.0); // anti-windup
  deriv = (err - err_prev) / DT;
  float u_pid = kp * err + ki * integ + kd * deriv; // salida PID base

  // ===============================================================
  // GAIN SCHEDULING (Reglas expertas suavizadas)
  // ===============================================================
  float gs = 1.0;

  if (fabs(err) > 0.8)       gs = 1.0;   // lejos del setpoint → PID completo
  else if (fabs(err) > 0.4)  gs = 0.75;  // medio
  else if (fabs(err) > 0.2)  gs = 0.73;  // cerca
  else if (fabs(err) > 0.05) gs = 0.60;  // muy cerca
  else                       gs = 0.61;  // casi en equilibrio

  u = gs * u_pid;

  // --- 3. Salida PWM ---
  float vOut = constrain(u, 0.0, 5.0);
  int pwm = map(vOut * 100, 0, 500, 0, 255);
  analogWrite(PIN_PWM, pwm);

  // --- 4. Telemetría ---
  if (PRINT_VERBOSE) {
    Serial.print(altura,3); Serial.print(',');
    Serial.print(setpoint,3); Serial.print(',');
    //Serial.print(u_pid,3); Serial.print(',');
    //Serial.print(gs,2); Serial.print(',');
    Serial.println(err);
  }

  // --- 5. Mantener periodo fijo ---
  t2 = micros();
  while ((t2 - t1) < DT_US) t2 = micros();

  err_prev = err;
}