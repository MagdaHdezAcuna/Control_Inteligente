// ===============================================================
// Levitador neumático con PID difuso (Fuzzy-PID)
// Sensor VL53L0X colocado arriba del objeto
// ===============================================================

#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// ----- SENSOR -----
Adafruit_VL53L0X lox;

// ----- PIN PWM -----
const int PIN_PWM = 9;

// ----- TIEMPOS -----
const unsigned long DT_US = 20000;
const float DT = DT_US * 1e-6;

// ----- RANGO -----
const float H_MIN = 0.0;
const float H_MAX = 45.0;
const float H_SENSOR = 51.0;

// ----- PID DIFUSO -----
float kp, ki, kd;      // Ganancias difusas (actualizadas en cada ciclo)
float e = 0, e_prev = 0;
float ce = 0;
float integ = 0;

// ----- SETPOINT -----
float variable = 30.0;


// ===============================================================
// FUNCIONES DE MEMBRESÍA TRIANGULARES
// ===============================================================
float tri(float x, float a, float b, float c) {
  if (x <= a || x >= c) return 0.0;
  if (x == b) return 1.0;
  if (x > a && x < b) return (x - a) / (b - a);
  return (c - x) / (c - b);
}

// ===============================================================
// EVALUACIÓN DEL CONTROL DIFUSO
// Entradas: e (error), ce (cambio error)
// Salidas: kp, ki, kd
// ===============================================================
void fuzzyPID(float e, float ce) {

  // --- NORMALIZACIÓN ---
  float eN  = constrain(e / 20.0, -1, 1);   // escala error a [-1,1]
  float ceN = constrain(ce / 20.0, -1, 1);

  // MF: NEG = -1 , ZERO = 0 , POS = 1
  float e_neg  = tri(eN, -1, -1, 0);
  float e_zero = tri(eN, -1, 0, 1);
  float e_pos  = tri(eN,  0, 1, 1);

  float ce_neg  = tri(ceN, -1, -1, 0);
  float ce_zero = tri(ceN, -1, 0, 1);
  float ce_pos  = tri(ceN,  0, 1, 1);

  // ---- Reglas tipo Mamdani (como tu tabla) ----
  // Salidas lingüísticas traducidas a valores numéricos:
  float KP_neg = 0.05;
  float KP_zero = 0.10;
  float KP_pos = 0.13;

  float KI_neg = 0.00;
  float KI_zero = 0.02;
  float KI_pos = 0.04;

  float KD_const = 0.03;

  // Activación de reglas
  float R[9];
  R[0] = min(e_neg,  ce_neg);
  R[1] = min(e_neg,  ce_zero);
  R[2] = min(e_neg,  ce_pos);
  R[3] = min(e_zero, ce_neg);
  R[4] = min(e_zero, ce_zero);
  R[5] = min(e_zero, ce_pos);
  R[6] = min(e_pos,  ce_neg);
  R[7] = min(e_pos,  ce_zero);
  R[8] = min(e_pos,  ce_pos);

  // KP por reglas
  float kp_out =
      R[0]*KP_neg + R[1]*KP_neg + R[2]*KP_neg +
      R[3]*KP_neg + R[4]*KP_pos + R[5]*KP_pos +
      R[6]*KP_pos + R[7]*KP_pos + R[8]*KP_pos;

  float sumK = R[0]+R[1]+R[2]+R[3]+R[4]+R[5]+R[6]+R[7]+R[8];
  if (sumK > 0) kp_out /= sumK;
  else kp_out = KP_zero;

  // KI por reglas
  float ki_out =
      R[0]*KI_zero + R[1]*KI_zero + R[2]*KI_zero +
      R[3]*KI_pos  + R[4]*KI_pos  + R[5]*KI_pos +
      R[6]*KI_zero + R[7]*KI_zero + R[8]*KI_zero;

  if (sumK > 0) ki_out /= sumK;
  else ki_out = KI_zero;

  kp = kp_out;
  ki = ki_out;
  kd = KD_const;  // fijo (opcional hacerlo difuso)
}

// ===============================================================
// SETUP
// ===============================================================
void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(PIN_PWM, OUTPUT);

  if (!lox.begin()) {
    Serial.println(F("ERROR sensor VL53L0X"));
    while (1);
  }
}

// ===============================================================
// LOOP PRINCIPAL (control)
// ===============================================================
void loop() {
  unsigned long t1 = micros();

  // --- LECTURA DE NUEVO SETPOINT DESDE SERIAL ---
  if (Serial.available() > 0) {
    float nuevoSP = Serial.parseFloat();
    if (nuevoSP > 0 && nuevoSP < 45) {  // límites de altura
      variable = nuevoSP;
      Serial.print("Nuevo setpoint: ");
      Serial.println(variable);
    }
    while (Serial.available()) Serial.read();  // limpiar buffer
  }

  // Lectura sensor
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  float altura = 0;
  if (measure.RangeStatus != 4) {
    float d = measure.RangeMilliMeter / 10.0;
    altura = constrain(H_SENSOR - d, H_MIN, H_MAX);
  }

  // Error
  float setpoint = variable + 5;
  e  = setpoint - altura;
  ce = e - e_prev;
  float err = variable - altura;

  // --- CONTROL DIFUSO ---
  fuzzyPID(e, ce);

  // PID usando ganancias difusas
  integ += e * DT;
  integ = constrain(integ, -80, 80);

  float deriv = ce / DT;
  float u = kp * e + ki * integ + kd * deriv;

  e_prev = e;

  // PWM
  int pwm = constrain(map(u * 100, 0, 500, 0, 255), 0, 255);
  analogWrite(PIN_PWM, pwm);

  // Telemetría hacia MATLAB
  Serial.print(altura); Serial.print(",");
  Serial.print(variable); Serial.print(",");
  Serial.print(err); Serial.print(",");
  Serial.print(kp, 3); Serial.print(",");
  Serial.print(ki, 3); Serial.print(",");
  Serial.print(kd, 3); Serial.print(",");
  Serial.println(pwm);

  // Mantener periodo de control
  while ((micros() - t1) < DT_US);
}
