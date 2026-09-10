// ===============================================================
// Levitador neumático - PID con reglas expertas
// Calibración polinómica (curva de Excel)
// Altura = f(voltage) usando polinomio de regresión
// ===============================================================

// ----- PINES -----
const int PIN_SHARP = A1;   // entrada analógica del sensor Sharp (0-5V)
const int PIN_PWM   = 9;    // salida PWM al ventilador (control velocidad)

// ----- TIEMPOS -----
const unsigned long DT_US = 20000; // periodo del lazo de control = 20 ms (50 Hz)
const float DT = DT_US * 1e-6;     // lo mismo expresado en segundos

// ----- RANGO -----
const float H_MIN = 0.0;   // altura mínima en cm
const float H_MAX = 40.0;  // altura máxima en cm

// ----- PID -----
float kp = 0.105;   // proporcional
float ki = 0.03;    // integral
float kd = 0.05;    // derivativo

float setpoint = 10.0; // altura deseada por defecto (10 cm)

// ----- VARIABLES DEL CONTROL -----
float altura = 0.0;      // altura medida (cm)
float err = 0.0;         // error = setpoint - altura
float err_prev = 0.0;    // error anterior
float integ = 0.0;       // parte integral acumulada
float deriv = 0.0;       // parte derivativa
float u = 0.0;           // salida del PID (voltios virtuales)

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

// Convierte voltaje medido -> altura (cm) con polinomio
float sharpVoltToCm(float v) {
  float d = 2.011*pow(v,5) -22.384*pow(v,4) + 95.135*pow(v,3) 
          -193.21*pow(v,2) + 200.14*v - 61.691;
  return constrain(d, H_MIN, H_MAX);
}

// ===============================================================
// CONFIGURACIÓN INICIAL
// ===============================================================
void setup() {
  Serial.begin(115200);
  pinMode(PIN_PWM, OUTPUT);
  analogWrite(PIN_PWM, 0);  // ventilador apagado al inicio

  Serial.println(F("LEVITADOR: inicio con PID + reglas expertas"));
  Serial.println(F("Envía por Serial un número (ej: 20) para cambiar el setpoint"));
  Serial.println(F("O escribe: PID kp ki kd   (ej: PID 0.08 0.02 0.01)"));
  Serial.println();
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
  t1 = micros();
  processSerial();

  // --- 1. Lectura del sensor ---
  int raw = analogRead(PIN_SHARP);
  float volt = adcToVolt(raw);
  altura = sharpVoltToCm(volt);

  // --- 2. Cálculo PID básico ---
  err = setpoint - (altura - 5); // compensación de offset
  integ += err * DT;
  integ = constrain(integ, -100.0, 100.0); // anti-windup
  deriv = (err - err_prev) / DT;
  u = kp * err + ki * integ + kd * deriv; // salida PID base

  // ==================================================
  // Reglas expertas
  // ==================================================
  if (fabs(err) > 0.8) {
    u = 0.45; //0.45
  } else if (fabs(err) > 0.40) {
    u = 0.40;
  } else if (fabs(err) > 0.20) {
    u = 0.12;
  } else if (fabs(err) > 0.01) {
    u = 0.10;
  }

  if ((err * deriv > 0) || (deriv == 0)) {
    if (fabs(err) > 0.05) {
      u = u + 2 * kp * err;
    } else {
      u = u + 0.4 * kp * err;
    }
  }

  if ((err * deriv < 0) && (deriv * (err - err_prev) > 0)) {
    u = u; // mantener
  }

  if ((err * deriv < 0) && (deriv * (err - err_prev) < 0)) {
    if (fabs(err) > 0.05) {
      u = u + 2 * kp * err;
    } else {
      u = u + 0.6 * kp * err;
    }
  }

  if (fabs(err) <= 0.001) {
    u = 0.5 * err + 0.010 * integ;
  }

  // --- 3. Salida PWM ---
  float vOut = constrain(u, 0.0, 5.0);
  int pwm = map(vOut * 100, 0, 500, 0, 255);
  analogWrite(PIN_PWM, pwm);

  // --- 4. Telemetría ---
  if (PRINT_VERBOSE) {
    Serial.print(raw); Serial.print(',');
    Serial.print(volt,4); Serial.print(',');
    Serial.print(altura,3); Serial.print(',');
    Serial.print(setpoint,3); Serial.print(',');
    Serial.print(vOut,3); Serial.print(',');
    Serial.println(pwm);
  }

  // --- 5. Mantener periodo fijo ---
  t2 = micros();
  while ((t2 - t1) < DT_US) t2 = micros();

  err_prev = err;
}
