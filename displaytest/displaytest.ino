#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <stdlib.h>
#define PhaseA 18
#define PhaseB 19

LiquidCrystal display(12, 11, 5, 4, 3, 2);
File root;
File entry; //모든 파일들을 담는 파일 배열 생성
File stage;

const int chipSelect = 53;
char state[2][8] = { "Stopped", "Playing" }; //재생상태 문자열을 담고있는 배열
char title_list[16][13];

unsigned char buffer[2][3000];
unsigned char fileheader[2][44];


void setup() {
    Serial.begin(115200);

    SD.begin(chipSelect);
    root = SD.open("/"); //파일 경로해당하는 파일을 연다

    stage = SD.open("JB_m844.WAV");
    entry = SD.open("JB_s822.WAV");

    stage.read(fileheader[0], 44);
    entry.read(fileheader[1], 44);

    Serial.println("stage     start");
    for (int i = 0; i < 44; i++) {
        Serial.print(fileheader[0][i], HEX);
        Serial.print("     ");
        Serial.println(fileheader[1][i],HEX);
    }

}

void loop() {
}


//stage test start
//52     52
//49     49
//46     46
//46     46
//BC     BC
//21     41
//85     A
//0     1
//57     57
//41     41
//56     56
//45     45
//66     66
//6D     6D
//74     74
//20     20
//10     10
//0     0
//0     0
//0     0
//1     1
//0     0 -> 여기까지 필요없는 정보
//1     2 -> 채널 수 정보 index no: 22
//0     0
//44     22 -> 샘플링 주파수 정보 index no: 24
//AC     56 -> 샘플링 주파수 정보
//0     0 x
//0     0 x
//44     88
//AC     58
//0     1
//0     0 -> byterate 안씀
//1     4
//0     0
//8     10 -> Bits per Sample index no: 34
//0     0
//64     64
//61     61
//74     74
//61     61
//0     0
//20     40
//85     A
//0     1


//if ((index_count + inter_check0) > 2996) {
//    index_count = 0;
//    inter_check0 = 1;
//}
//
//if ((index_count + inter_check1) > 2996) {
//    index_count = 0;
//    inter_check1 = 1;
//}
//OCR2B = (uint8_t)ptr[index_count];
//OCR2A = (uint8_t)ptr[index_count] + 0x80;
//OCR4B = (uint8_t)ptr[index_count];
//OCR4A = (uint8_t)ptr[index_count];
//
//index_count += 4;
//count = index_count;
//check0 = inter_check0;
//check1 = inter_check1;

