#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// ====== CONFIGURACIÓN DE PINES ======
const int PIN_PWM = 9;   // Salida PWM al ventilador

// ====== OBJETO SENSOR ======
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// ====== PARÁMETROS DE SENSOR ======
const float H_SENSOR = 45.0;   // Altura del sensor respecto a la base (cm)
const float H_MIN = 0.0;
const float H_MAX = 45.0;

// ====== VARIABLES ======
float altura = 0.0;
unsigned long t_prev = 0;
const unsigned long periodo = 50;   // 50 ms → 20 Hz

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(PIN_PWM, OUTPUT);
  analogWrite(PIN_PWM, 0);

  Serial.println(F("LEVITADOR NEUMÁTICO - VL53L0X + PWM (3.6–4.0 V)"));

  if (!lox.begin()) {
    Serial.println(F("ERROR: No se detecta el VL53L0X. Verifica las conexiones SDA=A4, SCL=A5."));
    while (1);  // Se queda detenido si no hay sensor
  }

  Serial.println(F("Sensor VL53L0X inicializado correctamente."));
}

// ====== LOOP ======
void loop() {
  // --- 1. Lectura del sensor cada 50 ms ---
  unsigned long t_now = millis();

  if (t_now - t_prev >= periodo) {
    t_prev = t_now;

    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);

    if (measure.RangeStatus != 4) {  // Si la medición es válida
      float d_sensor = measure.RangeMilliMeter / 10.0;  // mm → cm
      altura = H_SENSOR - d_sensor;
      altura = constrain(altura, H_MIN, H_MAX);
    } else {
      altura = NAN;  // Lectura inválida
    }

    // --- 2. Enviar la altura a MATLAB ---
    Serial.println(altura +10);
  }

  // --- 3. Revisar si MATLAB envía PWM (0–255) ---
if (Serial.available() > 0) {
  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input.length() > 0) {
    int pwm = input.toInt();
    pwm = constrain(pwm, 0, 255);
    analogWrite(PIN_PWM, pwm);
  }
}

}
