//Configuracion de nucleos a utlizar
#if CONFIG_FREERTOS_UNICORE
//sI FreeRTOS esta configurado para usar un solo nucleo, se asigna el nucleo 0
  static const BaseType_t app_cpu = 0;
#else
// Si se usan dos nucleos, se asigna el nucleo 1 
  static const BaseType_t app_cpu = 1;
#endif

int boton1 = 13;
int boton2 = 12;
int estado = 0;   // Estado de la secuencia
unsigned long tiempoInicio = 0;

void botones(void *parameter) {
  bool b1 = digitalRead(boton1); // HIGH = suelto, LOW = presionado
  bool b2 = digitalRead(boton2);
  while (1) {
    digitalWrite(led_pin1, HIGH);
    vTaskDelay(pdMS_TO_TICKS(500));
    digitalWrite(led_pin1, LOW);
    vTaskDelay(pdMS_TO_TICKS(500));
  }

}

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
