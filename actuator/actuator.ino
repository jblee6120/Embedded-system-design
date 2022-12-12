void setup() {
    Serial.begin(115200);
    DDRH |= _BV(PH0);
    DDRD |= _BV(PD0);
    DDRD |= _BV(PD1);
    DDRD |= _BV(PD2);
    DDRD |= _BV(PD3);

    PORTH &= ~_BV(PH0); // Motor Driver 작동 시작
    PORTD &= ~_BV(PD1);  // Motor Driver Direction 조정 0 : CW , 1 : CCW
    PORTD |= _BV(PD3);  // Motor Driver Direction 조정 0 : CW , 1 : CCW
    PORTD &= ~_BV(PD0); // step1의 시작값을 0으로 설정
    PORTD &= ~_BV(PD2); // step2의 시작값을 0으로 설정
}

void loop() {
    PORTD &= ~_BV(PD0); // step1의 시작값을 0으로 설정
    PORTD &= ~_BV(PD2); // step2의 시작값을 0으로 설정
    Serial.println("0 pulse");
    delay(10);

    PORTD |= _BV(PD0); // step1의 시작값을 0으로 설정
    PORTD |= _BV(PD2); // step2의 시작값을 0으로 설정
    Serial.println("1 pulse");
    delay(10);
}

void Timer_Init()
{
    TCCR1B &= ~_BV(WGM13);
    TCCR1B |= _BV(WGM12);
    TCCR1A &= ~_BV(WGM11);
    TCCR1A &= ~_BV(WGM10);  // CTC mode

    TCCR1B &= ~_BV(CS12);
    TCCR1B |= _BV(CS11);
    TCCR1B &= ~_BV(CS10); // set timer1 prescaler 8
    OCR1A = 99; // OCR1A at 20000kHz Carriar freq
    TCNT1 = 0;

    TIMSK1 |= _BV(OCIE1A);
    TIFR1 |= _BV(OCF1A);

    sei();
}

