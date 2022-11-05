#include <SPI.h>
#include <SD.h>

int i = 0;
int buffer[100];
const int chipSelect = 53;
void setup() {
	Serial.begin(115200);

	Serial.print("initializing SD card...");

	if (!SD.begin(chipSelect)) {
		Serial.println("Card failed, or not present");

		return;
	}
	Serial.println("card initialized.");

	File dataFile = SD.open("JB_M844.WAV");

	if (dataFile) {
		while (dataFile.available()) {
			buffer[i] = dataFile.read();
			Serial.println(buffer[i]);
			i++;
		}
		dataFile.close();
	}
	else {
		Serial.println("error opening...");
	}

}

void loop() {

}

