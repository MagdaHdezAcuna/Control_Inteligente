int boton1 = 13;
int boton2 = 12;
int estado = 0;   // Estado de la secuencia
unsigned long tiempoInicio = 0;

void setup() {
  pinMode(boton1, INPUT_PULLUP);
  pinMode(boton2, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  bool b1 = digitalRead(boton1); // HIGH = suelto, LOW = presionado
  bool b2 = digitalRead(boton2);

  switch (estado) {
    case 0: // Esperar presionar boton1
      if (b1 == LOW) {
        tiempoInicio = millis();
        estado = 1;
        Serial.println("Paso 1: Boton1 presionado");
      }
      break;

    case 1: // Esperar 1s después de presionar boton1
      if (millis() - tiempoInicio >= 1000) {
        if (b2 == LOW) { // Presionar boton2
          tiempoInicio = millis();
          estado = 2;
          Serial.println("Paso 2: Boton2 presionado");
        }
      }
      break;

    case 2: // Esperar 1s y luego soltar boton2
      if (millis() - tiempoInicio >= 1000) {
        if (b2 == HIGH) {
          tiempoInicio = millis();
          estado = 3;
          Serial.println("Paso 3: Boton2 liberado");
        }
      }
      break;

    case 3: // Esperar 1s y luego soltar boton1
      if (millis() - tiempoInicio >= 1000) {
        if (b1 == HIGH) {
          tiempoInicio = millis();
          estado = 4;
          Serial.println("Paso 4: Boton1 liberado");
        }
      }
      break;

    case 4: // Esperar >= 1s después de soltar boton1
      if (millis() - tiempoInicio >= 1000) {
        Serial.println(" Secuencia COMPLETA");
        estado = 0; // Reiniciar
      }
      break;
  }

  //  Resetear si no se cumple el orden
  if (estado > 0) {
    if ((estado == 1 && b1 == HIGH) ||   // Se soltó boton1 antes de tiempo
        (estado == 2 && b1 == HIGH) ||   // Se soltó boton1 en lugar de boton2
        (estado == 3 && b2 == LOW)) {    // Se volvió a presionar boton2
      Serial.println(" Secuencia incorrecta -> Reinicio");
      estado = 0;
    }
  }
}
