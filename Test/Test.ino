#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
File root;
File test;
File entry[16];
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int chipSelect = 53;

void readfile(File dir);

void setup() {
	lcd.begin(16, 2);
	Serial.begin(115200);
	
	SD.begin(chipSelect);
	root = SD.open("/");
	//entry[0] = root.openNextFile();
	//entry[1] = root.openNextFile();
	//test = root.openNextFile();

	readfile(root);

	Serial.println("test start");
	for (int i; i < 16; i++) {
		Serial.println(entry[i].name());
	}
	//Serial.println(test.name());
}

void loop(){
	for (int i = 0; i < 16; i++) {
		lcd.setCursor(0, 0);
		lcd.print(entry[i].name());
		delay(20);
	}
}

void readfile(File dir) {
	dir.rewindDirectory();

	for (int i = 0; i < 16; i++) {
		entry[i] = dir.openNextFile();
	}
} asdfsadf