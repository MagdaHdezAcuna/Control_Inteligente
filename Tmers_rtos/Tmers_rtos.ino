#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static TimerHandle_t auto_reload_timer1= NULL;
static TimerHandle_t auto_reload_timer2= NULL;
static TimerHandle_t auto_reload_timer3= NULL;

static TimerHandle_t auto_reload_timerR= NULL;
static TimerHandle_t auto_reload_timerA= NULL;
static TimerHandle_t auto_reload_timerV= NULL;


void myTimerCallback1(TimerHandle_t xTimer){
  /*Timer 1 a espirado*/
  if((uint32_t)pvTimerGetTimerID(xTimer)==1){
    Serial.print("Timer 1 \n");
  }
}

void myTimerCallback2(TimerHandle_t xTimer){
  /*Timer 1 a espirado*/
  if((uint32_t)pvTimerGetTimerID(xTimer)==1){
    Serial.print("Timer 2 \n");
  }
}

void myTimerCallback3(TimerHandle_t xTimer){
  /*Timer 1 a espirado*/
  if((uint32_t)pvTimerGetTimerID(xTimer)==1){
    Serial.print("Timer 3 \n");
  }
}

void setup(){
  Serial.begin(115200);
  /*---Creación de timers---*/
  auto_reload_timer1 =xTimerCreate(
                  "Timer Auto-reload",      //Nombre de Timer
                  2000/ portTICK_PERIOD_MS, //Periodo del Timer
                  pdTRUE,                   //Auto-reload
                  (void*)1,                 //ID Timer
                  myTimerCallback1);         //Función que manda a llamar
  
  auto_reload_timer2 =xTimerCreate(
                  "Timer Auto-reload",      //Nombre de Timer
                  2000 / portTICK_PERIOD_MS, //Periodo del Timer
                  pdTRUE,                   //Auto-reload
                  (void*)1,                 //ID Timer
                  myTimerCallback2);         //Función que manda a llamar
 
  auto_reload_timer3 =xTimerCreate(
                  "Timer Auto-reload",      //Nombre de Timer
                  2000/ portTICK_PERIOD_MS, //Periodo del Timer
                  pdTRUE,                   //Auto-reload
                  (void*)1,                 //ID Timer
                  myTimerCallback3);         //Función que manda a llamar
  
  auto_reload_timerR =xTimerCreate(
                  "Timer Auto-reload",      //Nombre de Timer
                  2000/ portTICK_PERIOD_MS, //Periodo del Timer
                  pdTRUE,                   //Auto-reload
                  (void*)1,                 //ID Timer
                  myTimerCallbackR);         //Función que manda a llamar

  auto_reload_timerA =xTimerCreate(
                  "Timer Auto-reload",      //Nombre de Timer
                  2000/ portTICK_PERIOD_MS, //Periodo del Timer
                  pdTRUE,                   //Auto-reload
                  (void*)1,                 //ID Timer
                  myTimerCallbackA);         //Función que manda a llamar
  auto_reload_timerV =xTimerCreate(
                  "Timer Auto-reload",      //Nombre de Timer
                  2000/ portTICK_PERIOD_MS, //Periodo del Timer
                  pdTRUE,                   //Auto-reload
                  (void*)1,                 //ID Timer
                  myTimerCallbackV);         //Función que manda a llamar

    xTimerStart(auto_reload_timer1, portMAX_DELAY);
}

void loop(){

}