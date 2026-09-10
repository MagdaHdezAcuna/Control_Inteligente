#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const int adc_pin = A0;
static hw_timer_t *timer =NULL;
static volatile uint16_t valor;
static SemaphoreHandle_t bin_sem = NULL;
/*1 MHZ*/
static const uint16_t timer_divisor =80;
static const uint16_t timer_max_contador =1000000;


void IRAM_ATTR onTimer(){
  /*Variable que permite saber si dentro de la interrupción se ha 
  activado una interrupción de mayor prioridad*/
  BaseTask_t task_woken =pdFalse; // Solo para ESP32
  /*if(task_woken){portYIELD_FROM();}*/
  //portYIELD_FROM(task_woken);//Para otro uM con FreeRTOS
  valor = analogREAD(adc_pin);
  //liberamos el semaforo para informar que hay un dato nuevo
  xSemaphoreGiveFromISR(bin_sem, &task_woken);
  //Salimos de interrupción
  if(task_woken ==pdTRUE){
    portYIELD_FROM_ISR();
  }
}

void printValores(void *parameters){
  while(1){
    /*Ciclo infinito de espera al semaforo e imprime el valor*/
    xSemaphoreTake(bin_sem, portMAX_DELAY);
    Serial.print("Valor es: ");
    Serial.println(valor);
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/ portTICK_PERIOD_MS);
 
  /*---Creación de semaforo binario---*/
  bin_sem= xSemaphoreCreateBinary();
  
  /*---Creación de tareas---*/  
  xTaskCreatePinnedToCore(printValores,
                          "Print Valores",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);
 
  /*---Creación de Timer número divisor, conteo ascendente---*/         
  timer =timerBegin(1000000);
 
  /*Interrupción temporizador timer, función, flanco*/  
  timerAttachInterrupt(timer, &onTimer); 
 
  /*Indicar a que cpmtep debe activarse la interrupción temporizador, conteo, autocarga*/  
  //tierAlarmWrite(timer, timer_max_contador, true);  
  //TimerAlarmEnable(timer);

}

void loop() {


}
