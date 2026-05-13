const byte pwmPin = 3;       // PWM 출력 핀: D3 = OC2B
const byte measurePin = 2;   // 주파수 측정 입력 핀

volatile unsigned long pulseCount = 0;

unsigned long previousMillis = 0;

const byte pwmTop = 103;     // 약 150Hz 설정용 TOP 값

void countPulse() {
  pulseCount++;
}

void setDutyPercent(float dutyPercent) {
  if (dutyPercent < 0) dutyPercent = 0;
  if (dutyPercent > 100) dutyPercent = 100;

  OCR2B = (byte)(pwmTop * dutyPercent / 100.0);
}

void setup() {
  Serial.begin(9600);

  pinMode(pwmPin, OUTPUT);
  pinMode(measurePin, INPUT);

  /*
    Timer2 설정
    Arduino Uno D3 = Timer2 OC2B

    Fast PWM mode
    TOP = OCR2A
    Prescaler = 1024

    PWM 주파수:
    f_PWM = 16MHz / (1024 * (1 + OCR2A))

    OCR2A = 103이면:
    f_PWM = 16,000,000 / (1024 * 104)
          ≈ 150.24Hz
  */

  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2  = 0;

  // Fast PWM, TOP = OCR2A
  TCCR2A |= (1 << WGM20) | (1 << WGM21);
  TCCR2B |= (1 << WGM22);

  // D3 = OC2B 비반전 PWM 출력
  TCCR2A |= (1 << COM2B1);

  // TOP 값 설정
  OCR2A = pwmTop;

  // 듀티비 설정
  // 기존 analogWrite(254)는 거의 100%에 가까운 듀티비였음
  // 여기서는 예시로 50% 설정
  setDutyPercent(10);

  // Timer2 시작: Prescaler = 1024
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);

  attachInterrupt(digitalPinToInterrupt(measurePin), countPulse, RISING);

  previousMillis = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 1000) {
    unsigned long elapsedTime = currentMillis - previousMillis;
    previousMillis = currentMillis;

    noInterrupts();
    unsigned long count = pulseCount;
    pulseCount = 0;
    interrupts();

    float frequency = count * 1000.0 / elapsedTime;

    Serial.print("Measured Frequency: ");
    Serial.print(frequency);
    Serial.println(" Hz");
  }
}