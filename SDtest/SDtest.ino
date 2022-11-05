#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#define PhaseA 18
#define PhaseB 19

LiquidCrystal display(12, 11, 5, 4, 3, 2);
File root;

const int chipSelect = 53;
void printDirectory(File dir, int numTabs);
char *filename[17][12];
char state[2][8] = { "Stopped", "Playing" }; //재생상태 문자열을 담고있는 배열

int i = 0;

volatile int cnt = 0;
volatile int play_state = 0;
void ext_int3_init();
void ext_int1_init();

void setup() {

    Serial.begin(115200);

//    Serial.print("\nInitializing SD card...");

    //if (!SD.begin(chipSelect)) {
    //    Serial.println("initialization failed.");

    //    return;
    //}
    //else {
    //    Serial.println("initialization done.");
    //}

    SD.begin(chipSelect);

    root = SD.open("/"); //파일 경로해당하는 파일을 연다

    printDirectory(root, 0);
    Serial.println("done!");
}

void loop() {

}

void printDirectory(File dir, int numTabs) {
    while (true) {
        File entry = dir.openNextFile(); //파일내부의 다음 파일을 객체에 저장

        if (!entry) break; //다음 파일이 없으면 끝난다.

        /*for (uint8_t i = 0; i < numTabs; i++) {
            Serial.print("\t");
        }*/

        *filename[i] = entry.name(); //현재 객체에 저장되어있는 파일의 이름을 출력한다
        Serial.println(*filename[i]);
        
        i++;
        //if (entry.isDirectory()) { //객체에 있는 파일이 디렉토리인 경우
        //    Serial.println("/"); //슬래시 출력
        //    printDirectory(entry, numTabs + 1); //다음 파일 출력
        //}

        //else {
        //    Serial.print("\t\t"); //디렉토리가 아닌 경우 즉, 파일인 경우 탭을 두 번 출력한다.
        //    //Serial.println(entry.size(), DEC); //파일의 크기를 출력한다
        //}
        //entry.close(); //객체에 있는 파일을 닫는다
    }
}