//Configuración de núcleos a utilizar
#if CONFIG_FREERTOS_UNICORE
 //si FreeRTOS está configurado para usar un solo núcleo, se asigna el núcleo 0
 static const BaseType_t app_cpu = 0;
#else
 //Si se usan dos núcleos, se asigna el núcleo 1
 static const BaseType_t app_cpu = 1;
#endif

static const int led_pin1 = 4;
static const int led_pin2 = 2;

void toggleLED (void *parameter){
  while (1){
  digitalWrite(led_pin1, HIGH);
  delay (500);
  digitalWrite(led_pin1, LOW);
  delay (500);
  }
 
}
void toggleLED2 (void *parameter){
  while (1){
  digitalWrite(led_pin2, HIGH);
  delay (323);
  digitalWrite(led_pin2, LOW);
  delay (323);
  }
 
}



void setup() {
  pinMode(led_pin1, OUTPUT);
  pinMode(led_pin2, OUTPUT);
  //creación de una tarea que se ejecutará de forma indefinida
  xTaskCreatePinnedToCore(  //En ESP32 se utilizaa esta función, en FreeRTOS normal sería XTaskCreate()
                        toggleLED,     //Función que implementa la tarea
                        "Toggle LED",  //Nombre descriptivoo de la función
                        1024,          //Tamaño de pila acignada
                        NULL,          //Parámetro a pasar de la función
                        1,             //Prioridad de la función 
                        NULL,          //Handle de la tarea
                        app_cpu);      //Núcleo en el se ejecutará la tarea (0 ó 1)
  xTaskCreatePinnedToCore(  //En ESP32 se utilizaa esta función, en FreeRTOS normal sería XTaskCreate()
                        toggleLED2,     //Función que implementa la tarea
                        "Toggle LED",  //Nombre descriptivoo de la función
                        1024,          //Tamaño de pila acignada
                        NULL,          //Parámetro a pasar de la función
                        1,             //Prioridad de la función 
                        NULL,          //Handle de la tarea
                        app_cpu);      //Núcleo en el se ejecutará la tarea (0 ó 1)
}

void loop() {
  // put your main code here, to run repeatedly:

}
