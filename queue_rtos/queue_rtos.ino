#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const uint8_t msg_queue_len = 5;

static QueueHandle_t msg_queue;

void MensajesUART(void *parameters){
  int item;
  while(1){
    //verificar si hay mensaje en la cola
    if(xQueueReceive(msg_queue, (void *)&item, 0)==pdTRUE){
    }
    Serial.print(item);
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  //creamos cola
  msg_queue = xQueueCreate(msg_queue_len, sizeof(int));
  xTaskCreatePinnedToCore(MensajesUART,
                          "MensajesUART",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

}

void loop() {
  static int num =  0;
  if(xQueueReceive(msg_queue, (void *)&num, 10) != pdTRUE){
    Serial.println("cola llena");
  }
  num++;
  vTaskDelay(500/portTICK_PERIOD_MS);

}
