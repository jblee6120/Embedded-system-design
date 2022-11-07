#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#define PhaseA 18
#define PhaseB 19

LiquidCrystal display(12, 11, 5, 4, 3, 2);
File root;
File entry[16]; //모든 파일들을 담는 파일 배열 생성
//개발 과정 중 파일명만 따로 뽑아서 출력하고, 한 개의 entry 클래스로
//cnt 값에 맞는 파일을 열어 entry 클래스에 넣고 파일명을 저장하는 배열에 저장하려 했음.
//하지만 이러한 방식은 파일 클래스가 제대로 저장되지 않고, 모든 배열에 파일 이름을 넣는 순간
//맨 마지막 파일명으로 배열의 모든 요소들이 자동으로 초기화되는 문제가 발생.
//따라서 모든 파일들을 각각의 클래스 배열에 넣어줘 파일들을 저장해서 cnt에 맞는 파일들을 그때그때 열 수 있고, 파일명도 성공적으로 출력할 수 있었음.


const int chipSelect = 53;
char state[2][8] = { "Stopped", "Playing" }; //재생상태 문자열을 담고있는 배열


int i = 0;

volatile int cnt = 0;
volatile int play_state = 0;
void ext_int3_init();
void ext_int1_init();
void readfile(File dir);


void setup() {
   Serial.begin(115200);

   DDRD &= ~0x0E; //PD3, PD2, PD1 을 INPUT으로 설정

   display.begin(16, 2); //2x16 display 설정

   ext_int3_init(); //INT3 interrupt 초기화
   ext_int1_init(); //INT1 interrupt 초기화
   sei(); //golbal interrupt enable 시작

   SD.begin(chipSelect);
   root = SD.open("/"); //파일 경로해당하는 파일을 연다
   readfile(root);
   for (int i = 0; i < 16; +i++) {
       Serial.print(entry[i].position()); //위치 출력용 코드
       Serial.println(entry[i].name());
   }
}

void loop() {
    display.setCursor(0, 0); //0번줄 0번째 칸에 커서 위치
    display.print("                "); //0번줄 전체 클리어
    display.setCursor(0, 0); //0번줄 0번째 칸에 커서 위치
    display.print(entry[cnt].name()); //각 파일 인스턴스의 이름 출력

    display.setCursor(0, 1); //1번줄 0번째 칸에 커서 위치
    display.print("                "); //1번줄 전체 클리어
    display.setCursor(0, 1); //0번줄 0번째 칸에 커서 위치
    display.print(state[play_state]); //재생상태 출력 구현 테스트, 나중에 재생상태 표시하는 함수로 다 묶어버리기
    /*파일 재생하는 함수 구현 필요!!!*/

    display.setCursor(0, 0);
    //Serial.println(print_title(1));
    delay(20); //display 유지를 위한 50ms동안의 delay.    
    
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

void readfile(File dir) {
    dir.rewindDirectory();
    dir.openNextFile();
    for (int i = 0; i < 16; i++) {
        entry[i] = dir.openNextFile();
    }
}

ISR(INT1_vect) {
    if (play_state == 0) play_state = 1;
    else play_state = 0;
}  

ISR(INT3_vect) {
    if (digitalRead(PhaseB)) cnt++;

    else cnt--;

    if (cnt >= 15) cnt = 0;

    else if (cnt <= 0) cnt = 0; //0번째 인스턴스는 SYSTEM 폴더이므로 처음에만 출력하고 나중에는 생략 이 부분 추가코드로 수정 필요
}