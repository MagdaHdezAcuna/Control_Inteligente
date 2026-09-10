// Declaración de variables globales 
float tempC;      
int pinLM35 = 0;  
bool transmitiendo = false; // Bandera para controlar el envío continuo
String comando = "";         // Variable para almacenar el texto recibido

void setup() {
  // Referencia interna a 1.1V para precisión del LM35
  analogReference(INTERNAL); 
  Serial.begin(9600);
}

void loop() {
  // Verificar si hay datos entrantes desde LabVIEW
  if (Serial.available() > 0) {
    comando = Serial.readStringUntil('\n'); // Lee hasta encontrar el salto de línea
    comando.trim(); // Elimina espacios en blanco o caracteres ocultos como \r

    if (comando == "Enviar") {
      transmitiendo = true;
    } 
    else if (comando == "Alto") {
      transmitiendo = false;
    }
  }

  // Si la bandera está activa, mide y envía los datos continuamente
  if (transmitiendo) {
    tempC = analogRead(pinLM35); 
    tempC = (1.1 * tempC * 100.0) / 1024.0; // Fórmula con ref de 1.1V

    // Formato solicitado: "Sensor 01: <valor>"
    //Serial.print("Sensor 01: ");
    //Serial.print(tempC); // Envía el valor y agrega el salto de línea (\n)
     Serial.print(tempC);
    //Salto de línea
     Serial.print("\n");

    delay(1000); // Retardo optimizado para que el sistema responda rápido (Modificable)
  }
}