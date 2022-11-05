//12181407 이종범 interrupt 과제
//Youtube Link: https://youtu.be/2advuJPtr34
//File name: interrupt_12181407.ino
//
//<설명>
//이 프로그램은 external interrupt를 이용하여 1000Hz의 PWM신호의 duty ratio를 계산한다
//초기설정은 50% duty ratio의 PWM을 external interrupt를 통해 계산하며,
//이후에는 Serial 통신으로 수신한 duty ratio값을 PWM에 입력해 입력된 duty ratio와 실제 계산된 duty ratio가 일치하는지 확인한다
//
//<duty ratio를 계산하는 방법>
//0. OC1A에 연결된 Pin18(PD3)의 출력을 Pin11(PB5)로 받는다.
//1. rising edge가 발생 시, external interrupt를 호출한다.
//2. ISR에서 OC1A에 연결된 pin11(PD3)이 1이면 pwm이 시작되었으므로 before_cnt에 TCNT1을 저장
//3. ISR에서 OC1A에 연결된 pin11(PD3)이 0이면 pwm출력이 Low가 되기 때문에 current_cnt에 TCNT1을 저장.
//4. ISR에서 이전 TCNT1과 현재 TCNT1과의 차이를 1주기 동안의 counter 값인 Top Value로 나눠주면 duty ratio가 나온다.
//5. 계산된 duty ratio 값을 Serial 통신으로 보내준다.
//6. 원하는 duty ratio는 Serial 통신으로 수신해서 input_duty에 저장해준다.
//7. 정수형 input_duty에 맞는 OCR1A에 들어갈 값으로 변환하려면 float형으로 변환하고 다시 정수형으로 변환한 뒤, OCR1A에 넣어준다.
//8. 계산된
// <Arduino MEGA 2560 - Peripheral Board 결선>
//		Arduino MEGA 2560	-	Peripheral Board
//			Pin18(PD3)				Pin11(PB5)

volatile int before_cnt = 0; //PWM이 시작될 때의 TCNT1값을 저장하는 변수
volatile int current_cnt = 0; //PWM의 excitation이 끝날 때의 TCNT1값을 저장하는 변수
// 위 두 변수는 ISR 에서 사용하는 변수이므로 volatile을 붙여서 컴파일러의 최적화를 막아준다.
uint32_t MicrosSampleTime; // micro 단위로 변환한 sample_time을 저장하는 변수
uint32_t start_time; //주기적 연산을 시작하는 시간을 저장하는 변수
float sample_time; //주기적 연산시간을 sec 단위로 저장하는 변수
void setup_pwm(); //PWM 설정 함수
void setup_extinterrupt(); //external interrupt 설정 함수
float input = 0; //Serial 통신으로 수신한 duty ratio를 저장하는 변수
float real_duty = 0; //아두이노에서 계산한 duty ratio를 저장하는 변수
int input_duty = 50; //duty ratio 맞는 OCR1A 값을 저장하는 변수

void setup() {

	Serial.begin(115200); //115200bps로 시리얼통신 시작
	
	sample_time = 0.02; //0.02초마다 주기적 연산을 한다.
	MicrosSampleTime = (uint32_t)(sample_time * 1e6); //0.02 sec를 micro단위로 변환

	DDRB |= 0b00100000; //Pin18(PB5)가 PWM 출력을 하기 위해 OUTPUT으로 설정
	DDRD &= ~0b00001000; //Pin18(PD3)에서 나온 PWM출력을 받기위해 Pin11(PD5)를 INPUT으로 설정
	
	setup_pwm(); //PWM 초기화
	setup_extinterrupt(); //External interrupt 초기화
	start_time = micros() + MicrosSampleTime; //주기적연산 시작시간 초기화
	sei(); //global interrupt enable 시작
}

ISR(INT3_vect) { //INT3를 vector로 갖는 Interrupt Service Routine
	if (PIND & 0b00001000) before_cnt = TCNT1; //PWM신호가 High일 경우 before_cnt에 TCNT1을 저장. -> 이전 시간을 저장
	
	else current_cnt = TCNT1; //PWM신호가 Low일 경우 current_cnt에 TCNT1을 저장 -> 현재 시간을 저장

	real_duty = (float)(current_cnt - before_cnt) * 100.0 / 1999.0; //현재 시간과 이전 시간을 비교해 계산된 duty ratio를 계산한다
}

void loop() {

	if (Serial.available()) {
		input_duty = Serial.parseInt(); //수신한 데이터 중 정수형만 추출해 변수에 저장한다
		input = (float)(input_duty)*1999/100; //duty ratio값과 일치하는 0CR1A을 구해서 저장한다.  
		cli(); //inerrupt 설정 변경 전에는 interrupt를 중단한다
		TCNT1 = 0; //counter값도 초기화한다
		OCR1A = (int)(input); //변경된 OCR1A값을 입력해준다
		sei(); //interrupt 시작
	}

	Serial.print(input_duty); //Serial통신으로 받은 duty ratio 출력
	Serial.print(" "); //데이터 구분은 스페이스로 구분한다
	Serial.println(real_duty); //실제로 계산된 duty ratio를 출력
	while (!((start_time - micros()) & 0x80000000)); //주기적 연산시간동안 대기
	start_time += MicrosSampleTime; //새로운 주기적연산 시작시간 갱신
}

void setup_pwm() {
	TCCR1A &= ~_BV(WGM10);
	TCCR1A |= _BV(WGM11);
	TCCR1B |= _BV(WGM12);
	TCCR1B |= _BV(WGM13); // WGM13 WGM12 WGM11 WGM10: 1 1 1 0 으로 setting 하여 ICR3을 Top Value로 사용하는 Fast PWM 모드 설정

	TCCR1A &= ~_BV(COM1A0);
	TCCR1A |= _BV(COM1A1); //COM1A1 COM1A0: 1 0 으로 setting하여 TCNT1이 OCR1A와 Compare Match 시 출력 핀이 LOW로 가도록 설정

	TCCR1B &= ~_BV(CS10);
	TCCR1B |= _BV(CS11);
	TCCR1B &= ~_BV(CS12); //CS12 CS11 CS10: 0 1 0으로 setting하여 1/8로 prescaling

	TCNT1 = 0; //TCNT1 초기화
	ICR1 = 1999; //1000Hz를 만드는 Top Value 설정
	OCR1A = 999; //50% duty ratio를 만드는 OCR1A 값 설정
}

void setup_extinterrupt() {
	EICRA |= _BV(ISC30); 
	EICRA &= ~_BV(ISC31); //ISC31 ISC30: 0 1로 setting하여 모든 edge에서 interrupt가 발생하도록 설정

	EIFR |= _BV(INTF3); //INT3의 flag를 0으로 초기화
	EIMSK |= _BV(INT3); //INT3 interrupt를 enable 시킨다
}

// 5%미만 95%를 초과하는 duty ratio를 계산하지 못하는 이유는 인터럽트가 호출되는 간격이 너무 짧아서 주어진 연산을 수행할 시간이 부족하다고 판단된다.