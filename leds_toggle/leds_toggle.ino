// Pines de salida
const int salida1 = 8;   // Pin digital para la salida 1
const int salida2 = 9;   // Pin digital para la salida 2
const int salida3 = 10;  // Pin digital para la salida 3

void setup() {
  pinMode(salida1, OUTPUT);
  pinMode(salida2, OUTPUT);
  pinMode(salida3, OUTPUT);

}

void loop() {
  // --- Toggle salida 1 (500 ms) ---
  digitalWrite(salida1, !digitalRead(salida1)); // Cambia estado
  delay(500); // Espera 500 ms

  // --- Toggle salida 2 (323 ms) ---
  digitalWrite(salida2, !digitalRead(salida2)); // Cambia estado
  delay(323); // Espera 323 ms

   // --- Toggle salida 3 (180 ms) ---
  digitalWrite(salida3, !digitalRead(salida3)); // Cambia estado
  delay(180); // Espera 323 ms
}