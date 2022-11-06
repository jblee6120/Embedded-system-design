char *filename[5][6];
String *test[5][6] = { "test1", "test2", "test3", "test4", "test5" };
void setup() {
	Serial.begin(115200);

	for (int i = 0; i < 5; i++) {
		*filename[i] = *test[i];
	}
	Serial.println("--------------");
	for (int i; i < 5; i++) {
		Serial.println(filename[i]);
		Serial.println(test[i]);
	}
}

void loop(){

}