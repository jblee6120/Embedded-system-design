#include <LiquidCrystal.h>
#define RANGE 1023
int data;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


void setup(){
	Serial.begin(115200);
	lcd.begin(16, 2);
	lcd.setCursor(0, 0);
	lcd.print("Port1 =");

}

void loop(){
	data = analogRead(A8);

	lcd.setCursor(8, 0);
	lcd.print((float)data / RANGE * 5.0);

	delay(100);

}
