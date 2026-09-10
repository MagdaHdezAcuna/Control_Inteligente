
static const TickType_t tiempo_pr = 200;

static void retardo (uint32_t ms){
  for(uint32_t i=0; i< ms; i++){
    for (uint32_t j=0;  j<40000; j++){
      asm("nop");
    }
  }
}

void TareaL(void *parameters){
  char str[20];
  while(1){
    sprintf(str,"Tarea Baja P, Núcleo %i\r\n", xPortGetCoreID());
    Serial.print(str);
    retardo(tiempo_pr);
  }
  
}

void TareaH(void *parameters){
  char str[20];
  while(1){
    sprintf(str,"Tarea Alta P, Núcleo %i\r\n", xPortGetCoreID());
    Serial.print(str);
    retardo(tiempo_pr); 
  }
  
}

void setup() {
  Serial.begin(115200);
  xTaskCreatePinnedToCore(TareaL,
                          "Tarea L",
                          2048,
                          NULL,
                          1,
                          NULL,
                          tskNO_AFFINITY);
   xTaskCreatePinnedToCore(TareaH,
                          "Tarea H",
                          2048,
                          NULL,
                          2,
                          NULL,
                          tskNO_AFFINITY);
}

void loop() {
  // put your main code here, to run repeatedly:

}
