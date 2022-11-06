#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#define PhaseA 18
#define PhaseB 19
LiquidCrystal display(12, 11, 5, 4, 3, 2);
File root;
File entry;

const int chipSelect = 53;
void file_name2buffer(File dir, int a);
void print_title(int a);
char filename[17][12];
char state[2][8] = { "Stopped", "Playing" }; //재생상태 문자열을 담고있는 배열

char* foo[17];


int i = 0;

volatile int cnt = 0;
volatile int play_state = 0;
void ext_int3_init();
void ext_int1_init();

void setup() {
   Serial.begin(115200);

   DDRD &= ~0x0E; //PD3, PD2, PD1 을 INPUT으로 설정

   display.begin(16, 2); //2x16 display 설정

   ext_int3_init(); //INT3 interrupt 초기화
   ext_int1_init(); //INT1 interrupt 초기화
   sei(); //golbal interrupt enable 시작

   SD.begin(chipSelect);

   root = SD.open("/"); //파일 경로해당하는 파일을 연다
   file_name2buffer(root, 16);
   Serial.println("--------------");
   for (int i = 0; i < 17; i++) Serial.println(*filename[i]);
   //entry = root.openNextFile();
   //*foo[0] = entry.name();
   //file_name2buffer(root);
}

void loop() {
    display.setCursor(0, 0); //0번줄 0번째 칸에 커서 위치
    display.print("                "); //0번줄 전체 클리어
    display.setCursor(0, 0); //0번줄 0번째 칸에 커서 위치
    
    //file_name2buffer(root, 0);
    
    //display.print(print_title(1)); // 노래 제목 출력 구현 테스트, 나중에 제목 출력하는 함수로 다 묶어버리기
    
    display.setCursor(0, 1); //1번줄 0번째 칸에 커서 위치
    display.print("                "); //1번줄 전체 클리어
    display.setCursor(0, 1); //0번줄 0번째 칸에 커서 위치
    display.print(state[play_state]); //재생상태 출력 구현 테스트, 나중에 재생상태 표시하는 함수로 다 묶어버리기

    display.setCursor(0, 0);
    //Serial.println(print_title(1));
    delay(20); //display 유지를 위한 50ms동안의 delay.    
    
}

void file_name2buffer(File dir, int a) {
    
    
    while (i<=a) {

        File entry = dir.openNextFile();

        //if (!entry) break; //다음 파일이 없으면 끝난다.

        filename[i] = (char)(entry.name()); //현재 객체에 저장되어있는 파일의 이름 저장한다
        Serial.println(*filename[i]);
        display.setCursor(0, 0);
        display.print(*filename[i]);
        delay(500);
        i++;
        //display.setCursor(0, 0);
        //display.print(*filename[0]);
        //delay(200);
    }
    Serial.println("------------------");
    for (int i = 0; i <= a; i++) Serial.println(*filename[i]);
}

void ext_int3_init() {
    EICRA |= _BV(ISC31);
    EICRA &= ~_BV(ISC30); //

    EIMSK |= _BV(INT3);
    EIFR |= _BV(INTF3);
}

void ext_int1_init() {
    EICRA &= ~_BV(ISC10);
    EICRA |= _BV(ISC11);

    EIMSK |= _BV(INT1);
    EIFR |= _BV(INTF1);
}

ISR(INT1_vect) {
    if (play_state == 0) play_state = 1;
    else play_state = 0;
}

ISR(INT3_vect) {
    if (digitalRead(PhaseB)) cnt++;

    else cnt--;

    if (cnt >= 17) cnt = 0;

    else if (cnt <= 0) cnt = 1;
}

void print_title(int a) {
    root.rewindDirectory();
    File title = root.openNextFile();
    for (int j = 0; j < a; j++) {
        title = root.openNextFile();
        //Serial.println(entry.name());
    }
    return title.name();
    //display.print(entry.name());
    //*foo[a] = entry.name();
    //Serial.println(*foo[a]);
}