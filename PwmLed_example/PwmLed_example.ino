float sample_time; //주기적 연산을 위한 샘플링타임 변수를 생성
float sim_time = 0.0; //사인파형의 시간값을 갖는 변수

uint32_t start_time; //주기적 연산을 시작하는 시작시간 저장 변수
uint32_t MicrosSampleTime; // 마이크로 단위의 샘플링타임을 저장하는 변수
uint16_t duty_value; //PWM의 duty ratio를 만들기 위한 duty값 변수 이게 OCR1A 즉, TOP 값이 됨.

void setup() {
	Serial.begin(115200); //시리얼 통신 시작
	sample_time = 0.02; //샘플링 타임은 0.02초
	MicrosSampleTime = (uint32_t)(sample_time * 1e6); //0.02초의 샘플링타임을 마이크로 단위로 변환

	DDRB |= 0b00100000;
	TCCR1B &= ~_BV(WGM13);
	TCCR1B |= _BV(WGM12);
	TCCR1A &= ~_BV(WGM11);
	TCCR1A |= _BV(WGM10); //WGM13 WGM12 WGM11 WGM10: 0 1 0 1로 설정해서 8 bit Fast PWM모드로 설정한다
	//Fast PWM 모드는 0~TOP까지 가면서 TOP값 도달 시 OC1A를 1로 set 한다.

	TCCR1A |= _BV(COM1A1);
	TCCR1A &= ~_BV(COM1A0); // COM1A1 COM1A0: 1 0으로 설정해서 compare match 발생 시 OC1A가 clear 되도록 설정

	TCCR1B &= ~_BV(CS12);
	TCCR1B |= _BV(CS11);
	TCCR1B |= _BV(CS10); // CS12 CS11 CS10: 0 1 1로 설정해서 presclaer 값을 64로 설정한다

	TCNT1 = 0; //카운터 값을 0으로 초기화한다

	start_time = micros() + MicrosSampleTime; //주기적 연산의 시작시간의 갱신은 현재시각+마이크로단위의 샘플링타임
}

void loop() {
	sim_time += sample_time; //매 주기마다 사인파에 들어갈 시간을 갱신
	duty_value = (uint16_t)(127.5 * sin(2 * sim_time) + 127.5); //pwm의 duty_value를 sine함수의 값을 받도록 한다. duty값의 자료형이 uint16_t(16bit timer counter를 사용하므로) 형변환을 하도록 한다.
	OCR1A = duty_value; //OCR1A의 값은 8bit fast pwm을 사용하므로 0~255까지 가능하다. TOP값이 255
	Serial.println(duty_value);

	while (!((start_time - micros()) & 0x80000000)); //주기적 연산이 끝날 때 까지 대기
	start_time += MicrosSampleTime; //시작시간 갱신

}