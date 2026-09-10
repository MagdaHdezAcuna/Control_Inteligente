// --- Configuración de PWM a 25 kHz para Convertidor CD-CD ---
// Microcontrolador: ATmega328P (Arduino Uno)
// Frecuencia de reloj: 16 MHz

const int pinPWM = 9; // Pin 9 = OCR1A (Timer1)

void setup() {
  pinMode(pinPWM, OUTPUT);

  // 1. Limpiar registros del Timer1
  TCCR1A = 0;
  TCCR1B = 0;

  // 2. Configurar Modo 14 (Fast PWM con ICR1 como TOP)
  TCCR1A |= (1 << COM1A1) | (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << WGM12);

  // 3. Prescaler = 1
  TCCR1B |= (1 << CS10);

  // 4. TOP para 25 kHz
  // f_PWM = 16 MHz / (Prescaler * (ICR1 + 1))
  // ICR1 = (16,000,000 / 25,000) - 1 = 639
  ICR1 = 533;

  // 5. Duty cycle inicial = 40%
  // OCR1A = 639 * 0.40 = 255.6
  OCR1A = 213;
}

void loop() {
  // PWM generado automáticamente por hardware
}