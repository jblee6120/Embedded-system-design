#include <SPI.h>
#include <SD.h>
File entry;
const int chipSelect = 53;

void readfile(int num);
unsigned char buffer[2][3000];

void information(File dir);
void pwm_init();
void interrupt_init();

volatile unsigned short a = 0;
volatile unsigned short count = 0x00;
short i = 0;
void setup() {
	DDRB |= 0x10;
	DDRH |= 0x58;
	TCCR1A &= ~_BV(WGM10);
	TCCR1A &= ~_BV(WGM11);

	TCCR1B |= _BV(WGM12);
	TCCR1B &= ~_BV(WGM13);

	TCCR1A &= ~_BV(COM1A0);
	TCCR1A &= ~_BV(COM1A1);

	TCCR1B |= _BV(CS10);
	TCCR1B &= ~_BV(CS11);
	TCCR1B &= ~_BV(CS12);

	OCR1A = 384;
	TCNT1 = 0;


	SD.begin(chipSelect);

	interrupt_init();
	information(entry);

	readfile(0);
	readfile(1);


	pwm_init();
	sei();
}

void loop() {
	if ((a==0) & (count == 2999)) {
		readfile(a);
	}

	else if ((a == 1) & (count == 2999)) {
		readfile(a);
	}

}

void readfile(int num) {
	entry.read(buffer[num],3000);
}


void information(File dir) {
	entry = SD.open("JB_s1622.WAV");
	for (int i = 0; i < 44; i++) {
		entry.read();
	}
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
	TCCR4B &= ~_BV(WGM43);
	TCCR4B |= _BV(WGM42);
	TCCR4A &= ~_BV(WGM41);
	TCCR4A &= ~_BV(WMG40);

	TCCR4A |= _BV(COM4A1);
	TCCR4A |= _BV(COM4A0);

	TCCR4B &= ~_BV(CS42);
	TCCR4B &= ~_BV(CS41);
	TCCR4B |= ~_BV(CS40);

	TCNT4 = 0; // TCNT4 clear

	TCNT2 = 0x00;
	TCNT4 = 0x00;

	OCR2A = 0x00;
	OCR2B = 0x00;
	OCR4A = 0x00;
	OCR4B = 0x00;
}

void interrupt_init() {
	TIMSK1 |= _BV(OCIE1A);
	TIFR1 |= _BV(OCF1A);
}

ISR(TIMER1_COMPA_vect) {
	if (count > 2999) {
		a++;
		count &= 0x00;
	}
	if (a == 2) a = 0;
	OCR2B = (buffer[a][count]);
	OCR2A = ((buffer[a][count + 1] + 0x80) & 0xFF);
	OCR4B = buffer[a][count + 2];
	OCR4A = (buffer[a][count + 3] + 0x80) & 0xFF;


	count +=1;
}