
# define pinButton 4
//Semáforo binario
SemaphoreHandle_t xSem = NULL;

void IRAM_ATTR isr_gpio(){
  BaseType_t task_woken = pdFALSE;
  xSemaphoreGiveFromISR(xSem, &task_woken);
  if (task_woken){
    portYIELD_FROM_ISR();
  }
}

void taskButton(void *parameter){
  while (1){
    if (xSemaphoreTake(xSem, portMAX_DELAY)){
      Serial.println("Cambio de estado detectado");
    }
  }
}

void setup(){
  Serial.begin(115200);

  //Crear semáforo
  xSem = xSemaphoreCreateBinary();

  //configurar pin con interrupción por flanco
  pinMode(pinButton, INPUT_PULLUP);
  attachInterrupt(pinButton, isr_gpio, CHANGE);

  //Crear tarea
  xTaskCreate(taskButton, "TaskButton", 2048, NULL, 1, NULL);
}

void loop(){

}