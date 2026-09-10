// Pines de botones
int boton1 = 7;
int boton2 = 6;

// Pines de salida
const int salida1 = 8;   
const int salida2 = 9;   
const int salida3 = 10;  

// Variables de control
int estado = 0;   
unsigned long tiempoInicio = 0;
bool secuenciaCompleta = false;  // bandera para activar salidas

// Control de tiempos de toggle
unsigned long t1 = 0, t2 = 0, t3 = 0;

void setup() {
  // Botones
  pinMode(boton1, INPUT_PULLUP);
  pinMode(boton2, INPUT_PULLUP);

  // Salidas
  pinMode(salida1, OUTPUT);
  pinMode(salida2, OUTPUT);
  pinMode(salida3, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  bool b1 = digitalRead(boton1); // HIGH = suelto, LOW = presionado
  bool b2 = digitalRead(boton2);

  // =====================
  // 1) MAQUINA DE ESTADOS
  // =====================
  switch (estado) {
    case 0: // Esperar presionar boton1
      if (b1 == LOW) {
        tiempoInicio = millis();
        estado = 1;
        Serial.println("Paso 1: Boton1 presionado");
      }
      break;

    case 1: // Esperar 1s y presionar boton2
      if (millis() - tiempoInicio >= 1000) {
        if (b2 == LOW) { 
          tiempoInicio = millis();
          estado = 2;
          Serial.println("Paso 2: Boton2 presionado");
        }
      }
      break;

    case 2: // Esperar 1s y soltar boton2
      if (millis() - tiempoInicio >= 1000) {
        if (b2 == HIGH) {
          tiempoInicio = millis();
          estado = 3;
          Serial.println("Paso 3: Boton2 liberado");
        }
      }
      break;

    case 3: // Esperar 1s y soltar boton1
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
        Serial.println("Secuencia COMPLETA");
        secuenciaCompleta = true; // activar salidas
        estado = 0; // reinicia para permitir otra secuencia
      }
      break;
  }

  // Resetear si se rompe la secuencia
  if (estado > 0) {
    if ((estado == 1 && b1 == HIGH) ||   // soltaron boton1 antes
        (estado == 2 && b1 == HIGH) ||   // soltaron boton1 en lugar de boton2
        (estado == 3 && b2 == LOW)) {    // presionaron boton2 de nuevo
      Serial.println("Secuencia incorrecta -> Reinicio");
      estado = 0;
    }
  }

  // =====================
  // 2) TOGGLE DE SALIDAS
  // =====================
  if (secuenciaCompleta) {
    unsigned long ahora = millis();

    // Salida1 (500ms)
    if (ahora - t1 >= 500) {
      digitalWrite(salida1, !digitalRead(salida1));
      t1 = ahora;
    }

    // Salida2 (323ms)
    if (ahora - t2 >= 323) {
      digitalWrite(salida2, !digitalRead(salida2));
      t2 = ahora;
    }

    // Salida3 (180ms)
    if (ahora - t3 >= 180) {
      digitalWrite(salida3, !digitalRead(salida3));
      t3 = ahora;
    }
  }
}