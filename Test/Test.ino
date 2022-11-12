#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
File root;
File test;
File entry;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int chipSelect = 53;

void readfile(File dir);
unsigned char buffer[1000];
unsigned char info[44];

void setup() {
	lcd.begin(16, 2);
	Serial.begin(115200);
	
	SD.begin(chipSelect);
	root = SD.open("/");
	//entry[0] = root.openNextFile();
	//entry[1] = root.openNextFile();
	//test = root.openNextFile();

	entry = SD.open("JB_s1622.WAV");
	entry.read(info, 44);




	readfile(root);

	Serial.println("test start");
	for (int i=0; i < 1000; i++) {
		Serial.println(buffer[i]);
	}
	//Serial.println(test.name());
}

void loop(){
}

void readfile(File dir) {
	entry.read(buffer, 1000);
	for (int i = 0; i < 1000; i+=2) {
		buffer[i] += 0x80;
	}

}