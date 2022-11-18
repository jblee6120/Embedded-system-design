#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <stdlib.h>

#define PhaseA 18
#define PhaseB 19

LiquidCrystal display(12, 11, 5, 4, 3, 2);
File root;
File stage;
//개발 과정 중 파일명만 따로 뽑아서 출력하고, 한 개의 entry 클래스로
//cnt 값에 맞는 파일을 열어 entry 클래스에 넣고 파일명을 저장하는 배열에 저장하려 했음.
//하지만 이러한 방식은 파일 클래스가 제대로 저장되지 않고, 모든 배열에 파일 이름을 넣는 순간
//맨 마지막 파일명으로 배열의 모든 요소들이 자동으로 초기화되는 문제가 발생.
//따라서 모든 파일들을 각각의 클래스 배열에 넣어줘 파일들을 저장해서 cnt에 맞는 파일들을 그때그때 열 수 있고, 파일명도 성공적으로 출력할 수 있었음.


const int chipSelect = 53;
char title_list[16][13];
char fileheader[44];

void readfile(File dir);
unsigned char buffer[2][3000];
unsigned char* ptr;
char* transfer_name2title;

void information();
void pwm_init();
void timer_init();

volatile unsigned short a = 0;
volatile unsigned short count = 0x00;
volatile char cnt = 0;
volatile char play_state = 0;
volatile unsigned char check0 = 0; //1이면 0번 버퍼 끝 0이면 0번버퍼 재생중
volatile unsigned char check1 = 0; //1이면 1번 버퍼 끝 0이면 1번버퍼 재생중
char choice = 1;
char change = 1;
char playing_mode = 0;

void ext_int3_init();
void ext_int1_init();
void readfile(File dir);


void setup() {
    Serial.begin(115200);

    DDRD &= ~0x0E; //PD3, PD2, PD1 을 INPUT으로 설정
    DDRB |= 0x10;
    DDRH |= 0x58;

    display.begin(16, 2); //2x16 display 설정

    ext_int3_init(); //INT3 interrupt 초기화
    ext_int1_init(); //INT1 interrupt 초기화
    sei(); //golbal interrupt enable 시작

    SD.begin(chipSelect);
    root = SD.open("/"); //파일 경로해당하는 파일을 연다
    readfile(root);

}

void loop() {
    Serial.println(OCR2A);
    switch (play_state) {

    case 0: //일시정지
        if (choice == 1) { //엔코더 사용 가능하게 설정
            TIMSK1 &= ~_BV(OCIE1A); //Timer interrupt disable
            ext_int3_init(); //엔코더 인터럽트 enable
            sei();
            choice = 0;
            change = 1;
        }
        else {
            display.clear();
            display.setCursor(0, 0); //0번줄 0번째 칸에 커서 위치
            display.print(title_list[cnt]); //각 파일 인스턴스의 이름 출력
            display.setCursor(0, 1); //0번줄 0번째 칸에 커서 위치
            display.print("STOPPED");
            delay(20);
        }
        break;

    case 1: //재생
        if (change == 1) {
            cli();
            information();
            display.setCursor(0, 1);
            display.print("Playing");
            stage.read(buffer[0], 3000);
            stage.read(buffer[1], 3000);
            EIMSK &= ~_BV(INT3);
            timer_init();
            pwm_init();
            sei();
            change = 0;
        }

        if (!(stage.available())) {
            play_state = 0;
            choice = 1;
        }
        switch (check0) {
        case 1:
            stage.read(buffer[0], 3000);
            check0 = 0;
            ptr = buffer[0];
        default:
            break;
        }

        switch (check1) {
        case 1:
            stage.read(buffer[1], 3000);
            check1 = 0;
            ptr = buffer[1];
        default:
            break;
        }
    default:
        break;
    }
    //display 유지를 위한 50ms동안의 delay.    

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

void information() {
    stage = SD.open(title_list[cnt]);
    stage.read(fileheader, 44);

    unsigned char channels = fileheader[22];
    unsigned char sample_rate = fileheader[24];
    unsigned char bits_per_sample = fileheader[34];

    if ((channels == 1) & (sample_rate == 0x22) & (bits_per_sample == 0x08)) {
        playing_mode = 0; //mono 22.05k 8bit
    }

    else if ((channels == 1) & (sample_rate == 0x22) & (bits_per_sample == 0x10)) {
        playing_mode = 1; //mono 22.05k 16bit
    }

    else if ((channels == 1) & (sample_rate == 0x44) & (bits_per_sample == 0x08)) {
        playing_mode = 2; //mono 44.1k 8bit
    }

    else if ((channels == 1) & (sample_rate == 0x44) & (bits_per_sample == 0x10)) {
        playing_mode = 3; //mono 44.1k 16bit
    }

    else if ((channels == 2) & (sample_rate == 0x22) & (bits_per_sample == 0x08)) {
        playing_mode = 4; //stereo 22.05k 8bit
    }

    else if ((channels == 2) & (sample_rate == 0x22) & (bits_per_sample == 0x10)) {
        playing_mode = 5; //stereo 22.05k 16bit
    }

    else if ((channels == 2) & (sample_rate == 0x44) & (bits_per_sample == 0x08)) {
        playing_mode = 6; //stereo 44.1k 8bit
    }

    else if ((channels == 2) & (sample_rate == 0x44) & (bits_per_sample == 0x10)) {
        playing_mode = 7; //stereo 44.1k 16bit
    }

}


void readfile(File dir) {
    root.openNextFile();
    for (int i = 0; i < 16; i++) {
        stage = root.openNextFile();
        transfer_name2title = stage.name();
        strcpy(title_list[i], transfer_name2title);
    }
}

void timer_init() {
    TCCR1A &= ~_BV(WGM10);
    TCCR1A &= ~_BV(WGM11);

    TCCR1B |= _BV(WGM12);
    TCCR1B &= ~_BV(WGM13);

    TCCR1A &= ~_BV(COM1A0);
    TCCR1A &= ~_BV(COM1A1);

    TCCR1B |= _BV(CS10);
    TCCR1B &= ~_BV(CS11);
    TCCR1B &= ~_BV(CS12);

    //OCR1A = 724;
    switch (fileheader[24]) {
    case 0x22:
        OCR1A = 724;
        break;

    case 0x44:
        OCR1A = 362;
        break;

    default:
        break;
    }

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
    TCCR4A &= 0b00001100;
    TCCR4A |= 0b10100001;
    TCCR4B &= 0b11100000;
    TCCR4B |= 0b00001001;

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
    if (play_state == 0) {
        play_state = 1;
        change = 1;
    }
    else {
        play_state = 0;
        choice = 1;
    }
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
    volatile char inter_mode = playing_mode;

    switch (inter_mode) {
    case 0:
        if ((index_count + inter_check0) > 2999) {
            index_count = 0;
            inter_check0 = 1;
            index_a++;
        }

        if ((index_count + inter_check1) > 2999) {
            index_a = 0;
            index_count = 0;
            inter_check1 = 1;
        }
        OCR2B = (uint8_t)ptr[index_count];
        OCR2A = (uint8_t)ptr[index_count];
        OCR4B = (uint8_t)ptr[index_count];
        OCR4A = (uint8_t)ptr[index_count];

        index_count += 1;
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;
        break;

    case 1:
        if ((index_count + inter_check0) > 2996) {
            index_count = 0;
            inter_check0 = 1;
        }

        if ((index_count + inter_check1) > 2996) {
            index_count = 0;
            inter_check1 = 1;
        }
        OCR2B = (uint8_t)ptr[index_count];
        OCR2A = (uint8_t)ptr[index_count + 1] + 0x80;
        OCR4B = (uint8_t)ptr[index_count];
        OCR4A = (uint8_t)ptr[index_count + 1] + 0x80;

        index_count += 2;
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    case 2:
        if ((index_count + inter_check0) > 2996) {
            index_count = 0;
            inter_check0 = 1;
        }

        if ((index_count + inter_check1) > 2996) {
            index_count = 0;
            inter_check1 = 1;
        }
        OCR2B = (uint8_t)ptr[index_count];
        OCR2A = (uint8_t)ptr[index_count];
        OCR4B = (uint8_t)ptr[index_count];
        OCR4A = (uint8_t)ptr[index_count];

        index_count += 1;
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    case 3:
        if ((index_count + inter_check0) > 2996) {
            index_count = 0;
            inter_check0 = 1;
        }

        if ((index_count + inter_check1) > 2996) {
            index_count = 0;
            inter_check1 = 1;
        }
        OCR2B = (uint8_t)ptr[index_count];
        OCR2A = (uint8_t)ptr[index_count + 1] + 0x80;
        OCR4B = (uint8_t)ptr[index_count];
        OCR4A = (uint8_t)ptr[index_count + 1] +0x80;

        index_count += 2;
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    case 4:
        if ((index_count + inter_check0) > 2996) {
            index_count = 0;
            inter_check0 = 1;
        }

        if ((index_count + inter_check1) > 2996) {
            index_count = 0;
            inter_check1 = 1;
        }
        OCR2B = (uint8_t)ptr[index_count];
        OCR2A = (uint8_t)ptr[index_count];
        OCR4B = (uint8_t)ptr[index_count + 1];
        OCR4A = (uint8_t)ptr[index_count + 1];

        index_count += 2;
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    case 5:
        if ((index_count + inter_check0) > 2996) {
            index_count = 0;
            inter_check0 = 1;
        }

        if ((index_count + inter_check1) > 2996) {
            index_count = 0;
            inter_check1 = 1;
        }
        OCR2B = (uint8_t)ptr[index_count];
        OCR2A = (uint8_t)ptr[index_count + 1] + 0x80;
        OCR4B = (uint8_t)ptr[index_count + 2];
        OCR4A = (uint8_t)ptr[index_count + 3] + 0x80;

        index_count += 4;
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    case 6:
        if ((index_count + inter_check0) > 2996) {
            index_count = 0;
            inter_check0 = 1;
        }

        if ((index_count + inter_check1) > 2996) {
            index_count = 0;
            inter_check1 = 1;
        }
        OCR2B = (uint8_t)ptr[index_count];
        OCR2A = (uint8_t)ptr[index_count];
        OCR4B = (uint8_t)ptr[index_count + 1];
        OCR4A = (uint8_t)ptr[index_count + 1];

        index_count += 2;
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    case 7:
        if ((index_count + inter_check0) > 2996) {
            index_count = 0;
            inter_check0 = 1;
        }

        if ((index_count + inter_check1) > 2996) {
            index_count = 0;
            inter_check1 = 1;
        }
        OCR2B = (uint8_t)ptr[index_count];
        OCR2A = (uint8_t)ptr[index_count + 1] + 0x80;
        OCR4B = (uint8_t)ptr[index_count + 2];
        OCR4A = (uint8_t)ptr[index_count + 3] + 0x80;

        index_count += 4;
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    default:
        break;
    }

   
}