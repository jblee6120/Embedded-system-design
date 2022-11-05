#define BUZZER_PIN 3 // 버저 핀 번호를 갖는 전처리기
volatile unsigned int cnt = 0; //software counter 구현을 위한 count 변수
volatile unsigned int TopValue = 100; //software counter 구현을 위한 topvalue 변수
volatile unsigned int OffTime = 5; //버저가 항상 켜져있을 순 없으므로 버저가 삑! 소리가 나면서 켜져있는 시간을 결정하는 변수
unsigned int period; //timer counter의 주기를 결정하는 OCR1A에 값을 입력하는 변수

void timer_init(); //timer interrupt를 사용하기 위한 함수 헤더

void setup() {
	timer_init(); //timer interrupt를 사용하기 위한 설정함수
	Serial.begin(115200); //115200bps로 시리얼통신 시작
	pinMode(BUZZER_PIN, OUTPUT); //buzzer pin의 모드를 출력으로 설정
	sei(); //global interrupt enable. 지금부터 interrupt가 시작된다.
}

ISR(TIMER1_COMPA_vect) { //interrupt service routine fuction. vector는 timer counter의 compare match 발생 시 flag가 1로 변한다
	cnt++; //cnt 증가
	if (cnt >= TopValue) cnt = 0; //top value 도달 시, cnt는 0으로 초기화

	if (cnt == 0) digitalWrite(BUZZER_PIN, HIGH); //cnt가 0이 되면 버저가울림
	if (cnt > OffTime)digitalWrite(BUZZER_PIN, LOW); //cnt가 0~5 즉, 0.05sec 동안 부저가 울리게 삑! 소리가 나도록 한다/
}

void loop() {
	if (Serial.available()) {
		period = Serial.parseInt(); //시리얼 통신으로 받은 값 중 정수를 뽑아서 변수에 입력
		cli(); //interrupt service 설정을 변경하기 전에 반드시 interrupt를 disable시켜야 한다. 그러므로 global interrupt enable을 disable 한다.
		TCNT1 = 0; //global inerrupt enable 하기 전, 쓰레기 값에 의한 오류를 방지하기 위해 counter 값이 TCNT1을 초기화 시켜준다.
		TopValue = period; //interrupt service routine의 설정 값 중 top value를 변경한다.
		sei(); //global interrupt enable 시작.
	}
}

void timer_init() {

	TCCR1A &= ~_BV(WGM11);
	TCCR1A &= ~_BV(WGM10);
	TCCR1B |= _BV(WGM12); //WGM13 WGM12 WGM11 WGM10: 0 1 0 0으로 세팅해서 Timer Counter를 CTC Mode로 동작하게 한다.
	TCCR1B &= ~_BV(WGM13);
	TCCR1B |= _BV(CS10);
	TCCR1B |= _BV(CS11);
	TCCR1B &= ~_BV(CS12); //CS12 CS11 CS10: 0 1 1 으로 설정해서 prescaling ratio를 1/64로 만든다. 

	OCR1A = 2499; //100Hz를 만들기 위한 OCR1A 값 설정
	TCNT1 = 0x0000; //counter value 초기화

	TIMSK1 |= _BV(OCIE1A); //Output Compare Match Interrupt Enable로 Comapare match 발생 시, interrupt flag가 발생하도록 한다.
	TIFR1 |= _BV(OCF1A); //interrupt flag를 초기화 하기 위해 1로 set한다.
}