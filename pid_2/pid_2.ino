// PID control for motor with encoder on ESP32 using FreeRTOS
// - TaskPID: control loop (2 ms)
// - TaskSerial: read setpoint from Serial and print debug
// Wiring: see comments above

#include <Arduino.h>


// ---------------------- PINOUT (ajusta según tu placa) ----------------------
const int PIN_ENCODER_A = 18;   // Encoder channel A
const int PIN_ENCODER_B = 19;   // Encoder channel B

// L298N pins (ENA = PWM, IN1/IN2 = direction)
const int PIN_PWM_EN = 25;      // ENA -> PWM (LEDC)
const int PIN_IN1     = 26;     // IN1
const int PIN_IN2     = 27;     // IN2

// ---------------------- LEDC PWM configuration ------------------------------
const int PWM_FREQ = 20000;     // 20 kHz
const int PWM_RES  = 8;         // 8-bit (0-255)
const int PWM_CHANNEL = 0;      // LEDC channel

// ---------------------- Model & Control params ------------------------------
volatile long Np = 0;           // Pulses counter (updated in ISR)

portMUX_TYPE muxEncoder = portMUX_INITIALIZER_UNLOCKED; // para ISR/crítico

// Conversión pulsos -> unidades (usa la que tú uses: grados o rad)
const float R = 0.1428f;        // resolución por pulso (ejemplo)
const float alpha = 0.05f;      // filtro derivada

// PID gains (del original)
const float kp = 1.25f;
const float kd = 0.25f;
const float ki = 0.07f;

// Variables compartidas (protegidas con mutex)
float th_des = 0.0f;            // consigna (setpoint)
float th_actual_debug = 0.0f;   // para imprimir desde TaskSerial
float pwm_debug = 0.0f;         // para imprimir

SemaphoreHandle_t xMutexConsigna = NULL;

// ---------------------- ISR del encoder (IRAM) ------------------------------
void IRAM_ATTR isrEncoderA() {
  // leer B para decidir incremento/decremento
  portENTER_CRITICAL_ISR(&muxEncoder);
  if (digitalRead(PIN_ENCODER_B) == LOW) Np++;
  else Np--;
  portEXIT_CRITICAL_ISR(&muxEncoder);
}

void IRAM_ATTR isrEncoderB() {
  portENTER_CRITICAL_ISR(&muxEncoder);
  if (digitalRead(PIN_ENCODER_A) == HIGH) Np++;
  else Np--;
  portEXIT_CRITICAL_ISR(&muxEncoder);
}

// ---------------------- Tarea PID (control) --------------------------------
void TaskPID(void *pvParameters) {
  (void) pvParameters;

  // Variables locales de la tarea
  float th = 0.0f, thp = 0.0f;
  float dth_d = 0.0f, dth_f = 0.0f;
  float e = 0.0f, de = 0.0f, inte = 0.0f;
  float u = 0.0f, usat = 0.0f;
  float PWMval = 0.0f;
  long Np_local = 0;

  const int dt_ms = 2;                 // 2 ms ~ 2000 us (igual que tu Arduino)
  const float dt = dt_ms * 0.001f;     // dt en segundos

  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) {
    // espera periódica determinista
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(dt_ms));

    // lectura rápida y segura del encoder
    portENTER_CRITICAL(&muxEncoder);
    Np_local = Np;
    portEXIT_CRITICAL(&muxEncoder);

    // Leer consigna (protegido por mutex)
    float target;
    if (xSemaphoreTake(xMutexConsigna, (TickType_t) 5) == pdTRUE) {
      target = th_des;
      xSemaphoreGive(xMutexConsigna);
    } else {
      target = th; // si falla mutex, mantener target previo
    }

    // Conversión y cálculo
    th = R * (float)Np_local;
    dth_d = (th - thp) / dt;
    dth_f = alpha * dth_d + (1.0f - alpha) * dth_f;

    // PID
    e = target - th;
    de = -dth_f;                   // derivada del error si setpoint constante
    inte = inte + (e * dt);

    // ANTI-WINDUP: limitar integral
    const float INTE_MAX = 100.0f; // ajusta según tu sistema
    if (inte > INTE_MAX) inte = INTE_MAX;
    if (inte < -INTE_MAX) inte = -INTE_MAX;

    u = kp*e + kd*de + ki*inte;

    // Saturación a voltaje +/-12V
    usat = constrain(u, -12.0f, 12.0f);

    // Escala a PWM 0-255 (aprox 255/12 = 21.25)
    PWMval = usat * 21.25f;
    int pwm_out = (int)fabs(PWMval);
    if (pwm_out > 255) pwm_out = 255;

    // Actuación en L298N:
    // - Si PWMval > 0: IN1 = HIGH, IN2 = LOW -> PWM en ENA
    // - Si PWMval < 0: IN1 = LOW, IN2 = HIGH -> PWM en ENA
    if (PWMval > 0.5f) {
      digitalWrite(PIN_IN1, HIGH);
      digitalWrite(PIN_IN2, LOW);
      ledcWrite(PWM_CHANNEL, pwm_out);
    } else if (PWMval < -0.5f) {
      digitalWrite(PIN_IN1, LOW);
      digitalWrite(PIN_IN2, HIGH);
      ledcWrite(PWM_CHANNEL, pwm_out);
    } else {
      // Stop / brake: poner PWM 0 y ambas entradas en LOW (o OPEN) según tu L298N
      ledcWrite(PWM_CHANNEL, 0);
      digitalWrite(PIN_IN1, LOW);
      digitalWrite(PIN_IN2, LOW);
    }

    thp = th;

    // actualizar variables para debug/imprimir
    th_actual_debug = th;
    pwm_debug = PWMval;
  }
}

// ---------------------- Tarea Serial (baja prioridad) ------------------------
void TaskSerial(void *pvParameters) {
  (void) pvParameters;
  String inputString = "";
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(20)); // cada 20 ms (no saturar Serial)

    // Lectura de Serial (consigna)
    if (Serial.available() > 0) {
      inputString = Serial.readStringUntil('\n');
      float nueva_cons = inputString.toFloat();
      if (xSemaphoreTake(xMutexConsigna, (TickType_t) 10) == pdTRUE) {
        th_des = nueva_cons;
        xSemaphoreGive(xMutexConsigna);
      }
    }

    // Imprimir valores para Plotter o debug
    // Formato: posición, pwm
    Serial.print(th_actual_debug, 4);
    Serial.print(",");
    Serial.println(pwm_debug, 4);
  }
}

// ---------------------- Setup y creación de tareas --------------------------
void setup() {
  // Serial
  Serial.begin(115200);
  delay(50);

  // Pines encoder
  pinMode(PIN_ENCODER_A, INPUT_PULLUP);
  pinMode(PIN_ENCODER_B, INPUT_PULLUP);

  // Pines L298N
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_PWM_EN, OUTPUT);

  // PWM (LEDC)
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
  ledcAttachPin(PIN_PWM_EN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0); // inicio 0

  // Interrupciones (usar digitalPinToInterrupt)
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_A), isrEncoderA, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_B), isrEncoderB, RISING);

  // Mutex para la consigna
  xMutexConsigna = xSemaphoreCreateMutex();
  if (xMutexConsigna == NULL) {
    Serial.println("ERROR: No se pudo crear mutex");
    while (1) delay(1000);
  }

  // Crear tareas (se fijan prioridades: PID > Serial)
  xTaskCreatePinnedToCore(
    TaskPID, "PID_Control", 4096, NULL, 3, NULL, 1
  );

  xTaskCreatePinnedToCore(
    TaskSerial, "Serial_Com", 4096, NULL, 1, NULL, 1
  );

  Serial.println("Sistema PID RTOS iniciado en ESP32");
}

void loop() {
  // No usamos loop, FreeRTOS maneja tareas. Para liberar recursos:
  vTaskDelete(NULL);
}
