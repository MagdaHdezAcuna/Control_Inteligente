//Configuracion de nucleos a utlizar
#if CONFIG_FREERTOS_UNICORE
//sI FreeRTOS esta configurado para usar un solo nucleo, se asigna el nucleo 0
  static const BaseType_t app_cpu = 0;
#else
// Si se usan dos nucleos, se asigna el nucleo 1 
  static const BaseType_t app_cpu = 1;
#endif

const char msg[]="Hola Mundo";


void tarea01(void *parameter){
  int msg_len strlen(msg);
  while(1){
    Serial.println();
    for(int i  =0; i<msg_len; i++){
      Serial.print(msg[i]);
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }
  Serial.println();
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void tarea01(void *parameter){
  while(1){
    for(int i =0; i<msg_len; i++){
      Serial.print();
      VTask(1000 / portTICK_PERIOD_MS);
    }
  }
}

void tarea02(void *parameter){
  while(1){
    Serial.print('+');
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}



void setup() {
  Serial.begin(300);
  vTaskDelay(500 /portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---Mensaje Inicial---");

 xTaskCreatePinnedToCore(// En ESP32 se utiliza esta funcion en FreeRTOS normal seria xTaskCreate()
                        tarea01,    // Funcion que implementa la tarea 
                        "Tarea 01", // Nombre descriptivo de la funcion 
                        1024,         // Tamaño de la pila asignada 
                        NULL,         // Parámetro a pasar de la funcion 
                        1,            // Prioridad de la función
                        NULL,         // Handle de la tarea 
                        app_cpu);     // Núcleo ene l se ejecutará la tarea (0 ó 1)

xTaskCreatePinnedToCore(// En ESP32 se utiliza esta funcion en FreeRTOS normal seria xTaskCreate()
                        tarea02,    // Funcion que implementa la tarea 
                        "Tarea 02", // Nombre descriptivo de la funcion 
                        1024,         // Tamaño de la pila asignada 
                        NULL,         // Parámetro a pasar de la funcion 
                        2,            // Prioridad de la función
                        NULL,         // Handle de la tarea 
                        app_cpu);     // Núcleo ene l se ejecutará la tarea (0 ó 1)



}

void loop() {
  /*for (int i=0; i<3; i++){
  vTaskSuspend(tarea_2); 
  vTaskDelay(2000 /portTICK_PERIOD_MS); 
  vTaskResume(tarea_2); 
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
 if(task_1 !=NULL){
  vTaskDelete(tarea_1);
  tarea_1 =NULL;
 }
  }*/

}