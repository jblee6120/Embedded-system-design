void setup() {
	DDRB |= 0b00100000;

#if 0
	TCCR1A &= 0b11111100; //WGM10 WGM11을 0으로 초기화
	TCCR1B &= 0b11100111; //WGM13 WGM12를 1로 초기화
	TCCR1B |= 0b00001000; //WGM12만 1로 set -> WGM13 WGM12 WGM11 WGM10: 0 1 0 0 으로 만들어서 CTC 모드 설정하기 위함

	TCCR1A &= 0b00111111; //COM1A0 COM1A1 0으로 초기화 
	TCCR1A |= 0b10000000; //COM1A1만 1로 set -> COM1A1 COM1A0: 1 0으로 설정해서 Top값과 compare match 발생 시 OC1A를 toggle시키는 모드 설정

	TCCR1B &= 0b11111000; //CS12 CS11 CS10을 0으로 초기화 
	TCCR1B |= 0b00000100; //CS12만 1로 set -> CS12 CS11 CS10: 1 0 0으로 설정해서 

#endif

#if 1
	TCCR1A &= ~_BV(WGM11); //
	TCCR1A &= ~_BV(WGM10);
	TCCR1A &= ~_BV(WGM13);
	TCCR1B |= _BV(WGM12);

	TCCR1A &= ~_BV(COM1A1);
	TCCR1A |= _BV(COM1A0);

	TCCR1B |= _BV(CS12);
	TCCR1B &= ~_BV(CS11);
	TCCR1B &= ~_BV(CS10);
#endif

	OCR1A = 31249;

	TCNT1 = 0;
}

void loop() {
}

