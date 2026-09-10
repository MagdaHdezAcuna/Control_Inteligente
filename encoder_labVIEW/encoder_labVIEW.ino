int dt_us = 2000;             // muestreo en micro-segundos
float dt = dt_us*0.000001;    // muestro en segundos
unsigned long t1 = 0, t2 = 0; // Tiempos para intervalos
unsigned long k = 0;          // Contador de muestras

int Np = 0;                   // Número de pulsos
const float R = 0.1428;       // Resolución de salida
float th = 0, thp =0;         // Posición angular y valor pasado
float dth_d = 0, dth_f =0;    // derivada discreta y s. filtrada
float alpha = 0.05;           // Coeficiente de filtro

// Parámetros PID (los que ya tenías calibrados)
float kp = 1.25, kd = 0.25, ki = 0.07;   
float e = 0, de = 0, inte = 0; 
float u = 0, usat = 0;        // SEÑAL DE CONTROL
float PWM = 0;                // SEÑAL PWM
float th_des = 0;             // CONSIGNA DEL CONTROL (SETPOINT)

// Pines L298N
const int sen1 = 5;
const int sen2 = 6;

String consigna;

void setup() {
  // Comunicación Serial
  Serial.begin(115200);
  // CRÍTICO: Reducir el timeout para no bloquear el lazo de control de 2ms
  Serial.setTimeout(2); 

  attachInterrupt(digitalPinToInterrupt(2), CH_A, RISING);
  attachInterrupt(digitalPinToInterrupt(3), CH_B, RISING);

  // Salidas de control PWM
  pinMode(sen1, OUTPUT);
  pinMode(sen2, OUTPUT);
}

void loop() {
  t1 = micros();  // Muestra de tiempo 1

  // ****************************************
  // 1. LEER CONSIGNA DE CONTROL DESDE LABVIEW
  if (Serial.available() > 0) {
    consigna = Serial.readStringUntil('\n');
    if (consigna.length() > 0) {
      th_des = consigna.toFloat();
      // Restringir los valores entre 0 y 360 por seguridad
      th_des = constrain(th_des, 0.0, 360.0);
    }
  }
      
  // 2. LECTURA DE SENSORES Y CÁLCULOS
  th = R * Np;            // Posición actual
  dth_d = (th - thp) / dt;  // Derivada discreta

  // Filtro de la velocidad 
  dth_f = alpha * dth_d + (1 - alpha) * dth_f;
      
  // 3. CONTROL PID
  e = th_des - th;             // ERROR 
  de = -dth_f;                 // DERIVADA DEL ERROR (evita el derivative kick)
  inte = inte + e * dt;        // INTEGRAL DEL ERROR
  
  u = kp * e + kd * de + ki * inte;  // VOLTAJE CALCULADO

  usat = constrain(u, -12, 12); // SATURACIÓN (Asumiendo fuente de 12V)
  
  // Convertir a PWM (0 a 255)
  PWM = usat * 21.25; // 255/12V = 21.25
      
  // 4. MANDAR SEÑAL DE CONTROL AL L298N
  if (PWM > 0) {
    analogWrite(sen1, PWM);
    analogWrite(sen2, 0);
  } else if (PWM < 0) {
    analogWrite(sen1, 0);
    // Multiplicar por -1 para obtener PWM positivo hacia el otro sentido
    analogWrite(sen2, -1 * PWM); 
  } else {
    analogWrite(sen1, 0);
    analogWrite(sen2, 0);
  }

  // 5. ACTUALIZAR VARIABLES Y ENVIAR DATOS
  k = k + 1;            // Número de muestra
  thp = th;             // Guardar valor de th para la siguiente iteración
      
  // Imprimir los valores para que LabVIEW los pueda leer (Opcional)
  Serial.print(th);
  Serial.print(',');
  Serial.println(PWM);

  // ****************************************
  
  // 6. CONTROL DEL TIEMPO DE MUESTREO (dt = 2000us)
  t2 = micros();  
  while ((t2 - t1) < dt_us) {
    t2 = micros();
  }
}

void CH_A() {
  if (digitalRead(3) == LOW)
    Np = Np + 1;
  else
    Np = Np - 1;
}

void CH_B() {
  if (digitalRead(2) == HIGH)
    Np = Np + 1;
  else
    Np = Np - 1;
}