#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const int led_pin =2;

static SemphoreHandle_t bin_sem;

void blinkLED(void *parameter){
  int num = *(int*)parameter;      //copiamos parametros en variable local
  //Liberamos el semaforo binario
  xSemaphoreGive(bin_sem);
  Serial.printil("Liberamos semaforo .......");
  pinMODE(led_led,OUTPUT);
  while(1){
    digitalWrite(led_pin, HIGH);
    vTaskDelay(num/portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(num/portTICK_PERIOD_MS);
  }
}


void setup(){
  long int delay_arg;
  Serial.begin(1125200);
  Serial.rpintln("Introduce el valor del retardo (milisegundos)");
  while (Serial.available()<=0);
  delay_arg = Serial.parseINT();
  
  Serial.println("creación de semaforo");
  Serial.println(delay_arg);

  bin_sem = XSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(blinkLED, 
                          "Blink LED",
                          (void *)&delay_arg,
                          1,
                          NULL,
                          app_cpu);

  xSemaphoreTake(bin_sem,portMAX_DELAY);
  Seral,println("Tomado ........");
}

void loop{

}