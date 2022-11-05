#include <LiquidCrystal.h>

#define BUZZER_PIN 6
unsigned int cnt = 0;
unsigned int sec = 0;
unsigned int min = 0;
unsigned int hour = 0;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void timer_init();

ISR(TIMER1_COMPA_vect) {
	cnt++;
	if (cnt >= 100) {
		cnt = 0;
		sec++;
	}

	if (sec >= 60) {
		sec = 0;
		min++;
	}

	if (min >= 60) {
		min = 0;
		hour++;
	}

	if (cnt == 0) digitalWrite(BUZZER_PIN, HIGH);
	if (cnt > 5)digitalWrite(BUZZER_PIN, LOW);
}

void setup() {
	pinMode(BUZZER_PIN, OUTPUT);
	lcd.begin(16, 2);
	lcd.setCursor(0, 0);
	lcd.print("Time");
	lcd.setCursor(2, 1);
	lcd.print(":");
	lcd.setCursor(5, 1);
	lcd.print(":");
	lcd.setCursor(8, 1);
	lcd.print(":");
	timer_init();
	sei();
}

void loop() {
	delay(100);
	lcd.setCursor(0, 1);
	lcd.print("  ");
	lcd.setCursor(0, 1);
	lcd.print(hour);
	lcd.setCursor(3, 1);
	lcd.print("  ");
	lcd.setCursor(3, 1);
	lcd.print(min);
	lcd.setCursor(6, 1);
	lcd.print("  ");
	lcd.setCursor(6, 1);
	lcd.print(sec);
	lcd.setCursor(9, 1);
	lcd.print("  ");
	lcd.setCursor(9, 1);
	lcd.print(cnt);
}

void timer_init() {
	TCCR1A &= ~_BV(WGM11);
	TCCR1A &= ~_BV(WGM10);
	TCCR1B |= _BV(WGM12);
	TCCR1B &= ~_BV(WGM13);
	TCCR1B |= _BV(CS10);
	TCCR1B |= _BV(CS11);
	TCCR1B &= ~_BV(CS12);

	OCR1A = 2499;
	TCNT1 = 0x0000;

	TIMSK1 |= _BV(OCIE1A);
	TIFR1 |= _BV(OCF1A);
}