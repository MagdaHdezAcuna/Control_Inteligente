//Declaración de variables globales 
float tempC; // variable para almacenear el valor obtneido del sensor (0 1023)
int pinLM35 = 0; // variable del pin de entrada del sensor (A0)

void setup() {
  // Cambiamos referencia de las entradas analógicas
  analogReference(INTERNAL);
  //Configuramos el puerto serial a 9600 bps
  Serial.begin(9600);
}

void loop() {
  // Con anaLogRead Leemos el sensor, recuerda que es un valor de 0 a 1023
  tempC = analogRead(pinLM35); 

  //Calculamos la temperatura con la fórmula
  tempC = (1.1*tempC*100.0)/1024.0;

  //Envia el dato al puerto serial
  Serial.print(tempC);
  //Salto de línea
  Serial.print("\n");

  //Espermaos un tiempo para repetir el loop
  delay(1000);
}
