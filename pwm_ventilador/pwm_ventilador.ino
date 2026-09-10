// === Configuración de PWM en pin 9 desde el Monitor Serial ===

const int PIN_PWM = 9;  // Pin de salida PWM

int valorPWM = 0;       // Valor del PWM (0-255)
String entrada = "";    // Cadena para leer el valor desde el Serial

void setup() {
  Serial.begin(115200);   // Inicia la comunicación serial
  pinMode(PIN_PWM, OUTPUT);
  Serial.println("Control PWM en pin 9");
  Serial.println("Escribe un valor entre 0 y 255 para ajustar el PWM:");
}

void loop() {
  // Verifica si hay datos disponibles en el monitor serial
  if (Serial.available() > 0) {
    entrada = Serial.readStringUntil('\n'); // Lee la línea completa
    entrada.trim(); // Elimina espacios o saltos extra

    // Convierte a número
    valorPWM = entrada.toInt();

    // Limita el valor entre 0 y 255
    valorPWM = constrain(valorPWM, 0, 255);

    // Aplica el valor al pin PWM
    analogWrite(PIN_PWM, valorPWM);

    // Muestra el resultado
    float voltaje = (valorPWM / 255.0) * 5.0;
    Serial.print("PWM: ");
    Serial.print(valorPWM);
    Serial.print("  -> Voltaje aprox: ");
    Serial.print(voltaje, 2);
    Serial.println(" V");
  }
}