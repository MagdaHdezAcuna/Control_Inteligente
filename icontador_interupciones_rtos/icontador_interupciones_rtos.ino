static const uint16_t divisor = 80;
static const uint64_t timer_max_count =1000000;
static const int led_pin =18;

static timer_t *timer =NULL;
//hw_timer_t *timer = timerBegin(1000000,divisor);

/*ISRs Rutina de Servicio de Interrupción*/
void IRAM_ATTR onTimer(){
  /*char str[20];
  sprintf(str, "Nucleo %i", xPortGetCoreID());
  Serial.println(str);*/
  int estado_pin =digitalRead(led_pin);
  digitalWrite(led_pin,!estado_pin);
}

void setup() {
  pinMode(led_pin, OUTPUT);
  /*cantidad, divisor, conteoacendente up/down*/
  timer = timerBegin(1000000, divisor);
  /*ISR (temporizador, función)*/
  timerAttachInterrupt(timer,&onTimer);
  /*(temporizador, contador, auto-recarga*/
  timerAlarm(timer,timer_max_count,true,0);
  //timerAlarmEnable(timer);

}

void loop() {
  // put your main code here, to run repeatedly:

}
