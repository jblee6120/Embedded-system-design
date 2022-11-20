#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#define PhaseA 18
#define PhaseB 19

LiquidCrystal display(12, 11, 5, 4, 3, 2);
File root;
File entry[16]; //모든 파일들을 담는 파일 배열 생성
File stage;
//개발 과정 중 파일명만 따로 뽑아서 출력하고, 한 개의 entry 클래스로
//cnt 값에 맞는 파일을 열어 entry 클래스에 넣고 파일명을 저장하는 배열에 저장하려 했음.
//하지만 이러한 방식은 파일 클래스가 제대로 저장되지 않고, 모든 배열에 파일 이름을 넣는 순간
//맨 마지막 파일명으로 배열의 모든 요소들이 자동으로 초기화되는 문제가 발생.
//따라서 모든 파일들을 각각의 클래스 배열에 넣어줘 파일들을 저장해서 cnt에 맞는 파일들을 그때그때 열 수 있고, 파일명도 성공적으로 출력할 수 있었음.


const int chipSelect = 53;
char state[2][8] = { "Stopped", "Playing" }; //재생상태 문자열을 담고있는 배열

void readfile(int num);
unsigned char buffer[2][3000];
unsigned char fileheader[44];

void information(File dir);
void pwm_init();
void timer_init();

volatile unsigned short a = 0;
volatile unsigned short count = 0x00;
volatile int cnt = 0;
volatile int play_state = 0;
volatile unsigned char check0 = 0; //1이면 0번 버퍼 끝 0이면 0번버퍼 재생중
volatile unsigned char check1 = 0; //1이면 1번 버퍼 끝 0이면 1번버퍼 재생중


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
}

void loop() {
    switch (play_state) {
    case 0:
        ext_int3_init();
        sei();
        break;

    case 1:
        EIMSK &= ~_BV(INT1);
        stage = entry[cnt];
        
    }
    display.setCursor(0, 0); //0번줄 0번째 칸에 커서 위치
    display.print("                "); //0번줄 전체 클리어
    display.setCursor(0, 0); //0번줄 0번째 칸에 커서 위치
    display.print(entry[cnt].name()); //각 파일 인스턴스의 이름 출력

    display.setCursor(0, 1); //1번줄 0번째 칸에 커서 위치
    display.print("                "); //1번줄 전체 클리어
    display.setCursor(0, 1); //0번줄 0번째 칸에 커서 위치
    display.print(state[play_state]); //재생상태 출력 구현 테스트, 나중에 재생상태 표시하는 함수로 다 묶어버리기

    display.setCursor(0, 0);

    switch (check0) {
    case 1:
        entry.read(buffer[0], 3000);
        check0 = 0;
    default:
        break;
    }

    switch (check1) {
    case 1:
        entry.read(buffer[1], 3000);
        check1 = 0;
    default:
        break;
    }

    delay(20); //display 유지를 위한 50ms동안의 delay.    
    
}


//인터럽트 초기화 함수, 여러가지 기능수행 함수내용
//Timer, external interrupt

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

void information(File dir) {
    entry[i].read(fileheader, 44);
 }


void readfile(File dir) {
    dir.rewindDirectory();
    dir.openNextFile();
    for (int i = 0; i < 4; i++) {
        entry[i] = dir.openNextFile();
    }
}

void timer_init() {
    DDRB |= 0x10;
    DDRH |= 0b01011000;
    TCCR1A &= ~_BV(WGM10);
    TCCR1A &= ~_BV(WGM11);

    TCCR1B |= _BV(WGM12);
    TCCR1B &= ~_BV(WGM13);

    TCCR1A &= ~_BV(COM1A0);
    TCCR1A &= ~_BV(COM1A1);

    TCCR1B |= _BV(CS10);
    TCCR1B &= ~_BV(CS11);
    TCCR1B &= ~_BV(CS12);

    OCR1A = 362;
    TCNT1 = 0;

    TIMSK1 |= _BV(OCIE1A);
    TIFR1 |= _BV(OCF1A);
}

void pwm_init() {
    //Timer counter 2 초기화
    TCCR2A |= _BV(COM2A1);
    TCCR2A &= ~_BV(COM2A0);

    TCCR2A |= _BV(COM2B1);
    TCCR2A &= ~_BV(COM2B0);

    TCCR2A |= _BV(WGM20);
    TCCR2A |= _BV(WGM21);
    TCCR2B &= ~_BV(WGM22);

    TCCR2B |= _BV(CS20);
    TCCR2B &= ~_BV(CS21);
    TCCR2B &= ~_BV(CS22);

    //Timer counter 4 초기화
    TCCR4B &= ~_BV(WGM43);
    TCCR4B |= _BV(WGM42);
    TCCR4A &= ~_BV(WGM41);
    TCCR4A &= ~_BV(WGM40);

    TCCR4A |= _BV(COM4A1);
    TCCR4A |= _BV(COM4A0);

    TCCR4B &= ~_BV(CS42);
    TCCR4B &= ~_BV(CS41);
    TCCR4B |= ~_BV(CS40);

    TCNT4 = 0; // TCNT4 clear

    TCNT2 = 0x00;
    TCNT4 = 0x00;

    OCR2A = 0x00;
    OCR2B = 0x00;
    OCR4A = 0x00;
    OCR4B = 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// interrupt service routine 정의

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

ISR(TIMER1_COMPA_vect) {
    volatile unsigned short index_a = a;
    volatile unsigned short index_count = count;
    volatile unsigned char inter_check0 = check0;
    volatile unsigned char inter_check1 = check1;
    if (index_count > 2996) {
        a++;
        index_count = 0;
        inter_check0 = 1;
    }
    if (index_a == 2) {
        a = 0;
        inter_check1 = 1;
    }


    OCR2B = buffer[index_a][index_count];
    OCR2A = buffer[index_a][index_count + 1];
    OCR4B = buffer[index_a][index_count];
    OCR4A = buffer[index_a][index_count + 1];

    index_count += 2;

    a = index_a;
    count = index_count;
    check0 = inter_check0;
    check1 = inter_check1;
}