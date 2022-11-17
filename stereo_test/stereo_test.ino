//16-bit, stereo WAV file 재생여부 테스트 파일

#include <SPI.h>
#include <SD.h>
File entry;
const int chipSelect = 53;

void readfile(int num);
unsigned char buffer[2][3000];
unsigned char fileheader[44];
unsigned char* ptr;

void information(File dir);
void pwm_init();
void interrupt_init();
void timer_init();

volatile unsigned char check0 = 0; //1이면 0번 버퍼 끝 0이면 0번버퍼 재생중
volatile unsigned char check1 = 0;
volatile unsigned short a = 0;
volatile unsigned short count = 0;


void setup() {
	Serial.begin(115200);
	SD.begin(chipSelect);

	interrupt_init();
	information(entry);
	for (int i = 0; i < 44; i++) Serial.println(fileheader[i], HEX);
	readfile(0);
	readfile(1);

	timer_init();
	pwm_init();
	sei();
}

void loop() {
	switch (check0) {
	case 1:
		entry.read(buffer[0], 3000);
		check0 = 0;
		ptr = buffer[0];

	default:
		break;
	}

	switch (check1) {
	case 1:
		entry.read(buffer[1], 3000);
		check1 = 0;
		ptr = buffer[1];

	default:
		break;
	}
	
	//entry.read(buffer[0], 3000);
	//while (!check0);
	//ptr = buffer[0];
	//check0 = 0;
	//
	//entry.read(buffer[1], 3000);
	//while (!check0);
	//ptr = buffer[1];
	//check0 = 0;
/////////// 중요!!! 버퍼 확인용 변수를 2개 사용해서 버퍼가 모두 동시에 쓰이는 것으로 보임
/////////// 그러므로 인터럽트 포함 모든 버퍼의 재생상태 확인용 변수를 하나로만 사용하고
/////////// 0 일때는 0번 버퍼 완료, 1번일때는 1번 버퍼 완료로 정하고 다시 switch문 작성이 필요함
	//switch (check1) {
	//case 1:
	//	entry.read(buffer[1], 3000);
	//	check1 = 0;
	//	ptr = buffer[1];
	//default:
	//	break;
	//}

	//if ((a == 0) & (count == 2999)) {
	//	entry.read(buffer[0], 3000);
	//}

	//else if ((a == 1) & (count == 2999)) {
	//	entry.read(buffer[1], 3000);
	//}

}

void readfile(int num) {
	entry.read(buffer[num], 3000);
}


void information(File dir) {
	entry = SD.open("JB_m844.WAV");
	entry.read(fileheader, 44);
}

void pwm_init() {
	//Timer counter 2 초기화
	TCCR2A |= _BV(COM2A1);
	TCCR2A &= ~_BV(COM2A0);

	TCCR2A |= _BV(COM2B1);
	TCCR2A &= ~_BV(COM2B0);

	TCCR2A |= _BV(WGM20);
	TCCR2A |= _BV(WGM21);
	TCCR2B &= ~_BV(WGM22);

	TCCR2B |= _BV(CS20);
	TCCR2B &= ~_BV(CS21);
	TCCR2B &= ~_BV(CS22);

	//Timer counter 4 초기화
	TCCR4A &= 0b00001100;
	TCCR4A |= 0b10100001;
	TCCR4B &= 0b11100000;
	TCCR4B |= 0b00001001;

	TCNT4 = 0; // TCNT4 clear

	TCNT2 = 0x00;

	OCR2A = 0x00;
	OCR2B = 0x00;
	OCR4A = 0x00;
	OCR4B = 0x00;
}

void interrupt_init() {
	TIMSK1 |= _BV(OCIE1A);
	TIFR1 |= _BV(OCF1A);
}

void timer_init() {
	DDRB |= 0x10;
	DDRH |= 0b01011000;
	TCCR1A &= ~_BV(WGM10);
	TCCR1A &= ~_BV(WGM11);

	TCCR1B |= _BV(WGM12);
	TCCR1B &= ~_BV(WGM13);

	TCCR1A &= ~_BV(COM1A0);
	TCCR1A &= ~_BV(COM1A1);

	TCCR1B |= _BV(CS10);
	TCCR1B &= ~_BV(CS11);
	TCCR1B &= ~_BV(CS12);

	OCR1A = 362;
	TCNT1 = 0;

}

ISR(TIMER1_COMPA_vect) {
	// 포인터 사용
	//volatile unsigned short index_a = a;
	//volatile unsigned short index_count = count;
	//volatile unsigned char inter_check0 = check0;
	//volatile unsigned char inter_check1 = check1;
	//if (index_count > 2996) {
	//	index_count = 0;
	//	inter_check0 = 1;
	//	index_a++;
	//}

	//if (index_a == 2) {
	//	index_a = 0;
	//	inter_check1 = 1;
	//}
	//OCR2B = (uint8_t)ptr[index_count];
	//OCR2A = (uint8_t)ptr[index_count];
	//OCR4B = (uint8_t)ptr[index_count];
	//OCR4A = (uint8_t)ptr[index_count];

	//index_count += 1;
	//
	//Serial.println(ptr[index_count]);
	//a = index_a;
	//count = index_count;
	//check0 = inter_check0;
	//check1 = inter_check1;

	//포인터 미사용
	volatile unsigned short index_a = a;
	volatile unsigned short index_count = count;
	volatile unsigned char inter_check0 = check0;
	volatile unsigned char inter_check1 = check1;
	if (index_count > 2996) {
		a++;
		index_count = 0;
		inter_check0 = 1;
	}
	if (index_a == 2) {
		a = 0;
		inter_check1 = 1;
	}


	OCR2B = buffer[index_a][index_count];
	OCR2A = buffer[index_a][index_count ];
	OCR4B = buffer[index_a][index_count];
	OCR4A = buffer[index_a][index_count];

	index_count += 1;

	a = index_a;
	count = index_count;
	check0 = inter_check0;
	check1 = inter_check1;

}