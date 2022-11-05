#include <LiquidCrystal.h>

#define PhaseA 18
#define PhaseB 19

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

volatile int cnt = 0;

void ExtInterruptInit();

ISR(INT3_vect) {
	if (digitalRead(PhaseB)) cnt--;
	else cnt++;
}

void setup() {
	pinMode(PhaseA, INPUT);
	pinMode(PhaseB, INPUT);

	//lcd.begin(16, 2);
	//lcd.setCursor(0, 0);
	//lcd.print("cnt");
	
	Serial.begin(115200);

	ExtInterruptInit();
	sei();


}

void loop() {
	//lcd.setCursor(0, 1);
	//lcd.print("   ");
	//lcd.setCursor(0, 1);
	//lcd.print(cnt);

	Serial.print(digitalRead(18));
	Serial.print(" ");
	Serial.print(digitalRead(19));
	Serial.print(" ");
	Serial.println(cnt);
}

void ExtInterruptInit() {
#if 0
	EICRA |= _BV(ISC30);
	EICRA |= _BV(ISC31);
#endif

#if 1
	EICRA &= ~_BV(ISC30);
	EICRA |= _BV(ISC31);
#endif

	EIFR |= _BV(INTF3);
	EIMSK |= _BV(INT3);
}

