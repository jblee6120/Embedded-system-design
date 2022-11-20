//12181407 이종범 Timer/Counter과제
//File name : WAV_File_Player_12181407.ino
//maker : 12181407 이종범
//Youtube Link: https://youtu.be/ES2Bs7EjZzg
// 
//주의사항 : 프로그램의 불안정성으로 인해 영상에서 나오는 큰 소리가 나오거나 갑자기 이상한 소리가 나올 수 있습니다.
//이 현상이 발생할 시, 리셋버튼을 눌러주시면 정상적으로 동작하는 모습을 확인할 수 있습니다.
//특히, 이 현상은 16bit stereo 음원을 재생할 때 자주 발생하는 문제입니다.그리고 스위치가 한 번에 여러 번 눌리게 되면,
//아두이노의 동작이 멈추는 현상이 발생하게 됩니다.이 또한 리셋버튼을 누르시면 해결되는 문제입니다.
//
//주요기능 : Arduino MEGA 2560을 이용하여 SD카드에 들어있는 WAV file data를 읽어 스피커로 노래를 재생하는 프로그램
//2 * 3000 Byte의 버퍼를 통해 음원 데이터를 읽어들여 PWM에 전송하는 방식으로 음악이 재생되며, sampling rate는 44.1k 22.05kHz 가 있다.
//그리고 한 샘플당 들어있는 비트 수는 8, 16bit 이며, 스피커의 채널 수는 mono의 경우 1channel, stero의 경우 2channels을 사용한다.
//이 모든 정보는 WAV file의 0~44 Byte에 들어있으며, 파일 헤더를 읽는 배열을 통해 필요한 정보를 정확하게 뽑아와 재생 모드를 결정한다.
//sampling rate에 맞게 timer interrupt를 변경해 정상적인 속도로 재생되는 음악을 들어볼 수 있다.
//
//노래를 선택하는 부분은 external interrupt를 이용해 엔코더를 읽어들여 배열에 저장된 문자열을 출력하고 스위치로 선택 시, 해당 파일명을 갖는 파일을 SD카드에서
//파일 헤더와 음원 데이터를 서로 다른 배열로 읽어들인다.
//
//재생 시, external interrupt는 encoder만 diable 시키고 음원 버퍼에 3000바이트씩 데이터를 받아와 timer interrupt 발생 시, 이 데이터를 PWM의 OCRnX에 보내줘 duty ratio를 조절한다.
//음원의 sampling rate가 최대 44.1kHz이므로 PWM은 그보다 큰 62.5kHz를 만들기 위해 8-bit pwm을 사용한다.
//
//정지는 스위치가 눌리는 인터럽트 발생 시, 재생상태를 나타내는 변수를 바꿔주면서 음원이 선택 가능하며, 화면을 보여줄 수 있으며, 음악을 멈추게 하는 기능을 차례대로 수행하도록 한다.

#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <stdlib.h> //문자열 저장을 위한 함수를 사용하기 위한 헤더파일

#define PhaseA 18
#define PhaseB 19

LiquidCrystal display(12, 11, 5, 4, 3, 2); //4선 방식의 lcd를 선언해준다.
File root; 
File stage; //파일을 담기 위한 File 클래스 생성

//개발 과정 중 파일명만 따로 뽑아서 출력하고, 한 개의 entry 클래스로
//cnt 값에 맞는 파일을 열어 entry 클래스에 넣고 파일명을 저장하는 배열에 저장하려 했음.
//하지만 이러한 방식은 파일 클래스가 제대로 저장되지 않고, 모든 배열에 파일 이름을 넣는 순간
//맨 마지막 파일명으로 배열의 모든 요소들이 자동으로 초기화되는 문제가 발생.
//따라서 모든 파일들을 각각의 클래스 배열에 넣어줘 파일들을 저장해서 cnt에 맞는 파일들을 그때그때 열 수 있고, 파일명도 성공적으로 출력할 수 있었음.
//하지만 실행 속도가 느려져서 문자열을 다시 사용하게 되었음.

const int chipSelect = 53; //SC핀 역할을 하는 53번 핀을 지정하기위한 변수
char title_list[16][13]; //파일명을 담아두는 배열 선언
char fileheader[44]; //WAV file의 파일 헤더를 담아두는 배열 선언

void readfile(File dir); //파일 명을 읽어들이는 함수
unsigned char buffer[2][3000]; //음악 재생을 위한 버퍼. 3000 Byte 버퍼 2개를 번갈아가면서 사용한다.
unsigned char* ptr; //버퍼의 주소를 가리키는 포인터. 포인터를 통해 버퍼의 인덱스를 1개만 사용가능함.
char* transfer_name2title; //파일명을 일시적으로 담고 파일명 배열에 넘겨주는 포인터

void information(); //파일의 정보를 읽어오고, 재생모드를 바꿔주는 함수
void pwm_init(); //PWM 초기화 함수
void timer_init(); //Timer interrupt 초기화 함수

volatile unsigned short count = 0x00; //버퍼의 인덱스를 나타내는 변수
volatile char cnt = 0; //엔코더에서 인터럽트 발생 시, 그 수를 세는 변수
volatile char play_state = 0; //음원 재생상태를 나타내는 변수 0이면 정지, 1이면 재생을 나타낸다.
volatile unsigned char check0 = 0; //1이면 0번 버퍼 끝 0이면 0번버퍼 재생중임을 나타내는 변수
volatile unsigned char check1 = 0; //1이면 1번 버퍼 끝 0이면 1번버퍼 재생중임을 나타내는 변수
char choice = 1; //음원선택, 엔코더의 외부 인터럽트 enable, 음악재생이 멈춘 상태이므로 timer interrupt가 정지되어야 하는 상태로 변경해야 함을 나타내는 변수
char change = 1; //음악 재생 시, 재생 모드를 설정해야 하는 상태임을 나타내주는 변수 1이면 모드 변경을 나타낸다
char playing_mode = 0; //음악의 8가지 재생모드를 나타내는 변수

void ext_int3_init(); //encoder에 연결된 int3 external interrupt 초기화 함수
void ext_int1_init(); //encoder sw에 연결된 int1 external interrupt 초기화 함수


void setup() {
    DDRD &= ~0x0E; //PD3, PD2, PD1 을 INPUT으로 설정
    DDRB |= 0x10;
    DDRH |= 0x58;

    display.begin(16, 2); //2x16 display 설정

    ext_int3_init(); //INT3 interrupt 초기화
    ext_int1_init(); //INT1 interrupt 초기화
    sei(); //golbal interrupt enable 시작

    SD.begin(chipSelect); //SC 53으로 SD카드 초기화
    root = SD.open("/"); //파일 경로해당하는 파일을 연다
    readfile(root); //root 경로를 통해 파일명을 저장하는 함수 실행

}

void loop() {
    switch (play_state) {

    case 0: //일시정지
        if (choice == 1) { //엔코더 사용 가능하게 설정
            TIMSK1 &= ~_BV(OCIE1A); //Timer interrupt disable
            ext_int3_init(); //엔코더 인터럽트 enable
            sei();
            choice = 0; //불필요한 설정을 막기 위해(재생 중에도) 0으로 바꿔준다
            change = 1; //재생모드를 바꾸기 위해 준비
        }
        else {
            display.clear();
            display.setCursor(0, 0); //0번줄 0번째 칸에 커서 위치
            display.print(title_list[cnt]); //각 파일 인스턴스의 이름 출력
            display.setCursor(0, 1); //0번줄 0번째 칸에 커서 위치
            display.print("STOPPED");
            delay(20); //일시정지 중에는 항상 화면이 띄워져 있어야 함. 그래서 20ms의 시간을 줌
        }
        break;

    case 1: //재생
        if (change == 1) { //음원 변경을 해야할 때. 정지 후 재생시
            cli(); //interrupt 변경을 위해 globla interrupt를 disable 시킨다
            information(); //파일 헤더를 읽어와서 재생모드를 결정
            display.setCursor(0, 1);
            display.print("Playing");
            stage.read(buffer[0], 3000); //파일을 읽기위한 버퍼를 미리 준비
            stage.read(buffer[1], 3000);
            EIMSK &= ~_BV(INT3); //재생 중에는 엔코더가 동작하면 안되므로 int3 interrupt를 disable 시킨다
            timer_init(); //재생 모드에 맞는 timer interrupt 초기화
            pwm_init(); //pwm 초기화
            sei(); //gloabal interrupt 시작
            change = 0; //모드 변경은 재생 중에는 할 필요가 없으므로 비활성화 시킨다
        }

        if (!(stage.available())) { //음원 재생이 끝났으면
            play_state = 0; //일시정지로 바꾸고
            choice = 1; //인터페이스 조작이 가능하도록 한다
        }
        switch (check0) {
        case 1: //0번 버퍼가 끝날 때
            stage.read(buffer[0], 3000); //3000 Byte 씩 채워넣는다
            check0 = 0; //0번 버퍼는 사용 중인 상태로 만든다
            ptr = buffer[0]; //재생용 포인터에 버퍼의 주소를 넣어준다
        default:
            break;
        }

        switch (check1) {
        case 1: //1번 버퍼가 끝날 때
            stage.read(buffer[1], 3000); //3000 Byte 씩 채워넣는다
            check1 = 0; //1번 버퍼는 사용 중인 상태로 만든다
            ptr = buffer[1]; //재생용 포인터에 버퍼의 주소를 넣어준다
        default:
            break;
        }
    default:
        break;
    }

}


//인터럽트 초기화 함수, 여러가지 기능수행 함수내용
//Timer, external interrupt

void ext_int3_init() {
    EICRA |= _BV(ISC31);
    EICRA &= ~_BV(ISC30);

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

    unsigned char channels = fileheader[22]; //22번 데이터가 사용하는 채널의 수
    unsigned char sample_rate = fileheader[24]; //24번 데이터가 sampling rate를 나타내기 시작 22.05khz는 0x22 44.1kHz는 0x44로 나타난다
    unsigned char bits_per_sample = fileheader[34]; //8-bit와 16-bit를 나타낸다

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
    for (int i = 0; i < 16; i++) { //넉넉하게 16개 정도의 파일의 이름을 읽어와 title_list에 파일명을 저장한다
        stage = root.openNextFile(); //파일을 하나씩 연다
        transfer_name2title = stage.name(); //파일명을 일시적으로 갖고 있는다
        strcpy(title_list[i], transfer_name2title); //파일명을 주어진 배열에 넣어준다
    }
}

void timer_init() {
    //Timer counter 1 초기화
    TCCR1A &= ~_BV(WGM10);
    TCCR1A &= ~_BV(WGM11);

    TCCR1B |= _BV(WGM12);
    TCCR1B &= ~_BV(WGM13);

    TCCR1A &= ~_BV(COM1A0);
    TCCR1A &= ~_BV(COM1A1);

    TCCR1B |= _BV(CS10);
    TCCR1B &= ~_BV(CS11);
    TCCR1B &= ~_BV(CS12);

    switch (fileheader[24]) { //sampling rate정보를 확인해서
    case 0x22: //22.05khz
        OCR1A = 724;
        break;

    case 0x44: //44.1khz
        OCR1A = 362;
        break;

    default:
        break;
    }

    TCNT1 = 0;

    TIMSK1 |= _BV(OCIE1A); //timer interrupt enable
    TIFR1 |= _BV(OCF1A); //flag 초기화
}

void pwm_init() {
    //Timer counter 2 초기화
    TCCR2A |= _BV(COM2A1);
    TCCR2A &= ~_BV(COM2A0); //compare match 시 OC2A는 clear

    TCCR2A |= _BV(COM2B1);
    TCCR2A &= ~_BV(COM2B0); //compare match 시 OC2B는 clear

    TCCR2A |= _BV(WGM20);
    TCCR2A |= _BV(WGM21);
    TCCR2B &= ~_BV(WGM22); //8-bit pwm mode로 설정

    TCCR2B |= _BV(CS20);
    TCCR2B &= ~_BV(CS21);
    TCCR2B &= ~_BV(CS22); //prescaling value는 1로 설정

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
    if (play_state == 0) { //정지 상태면
        play_state = 1; //재생상태로 바꾸고
        change = 1;
    }
    else {
        play_state = 0; //재생 중이면 정직로 바꿔준다
        choice = 1; //인터페이스 이용가능한 설정으로 진입하도록 만들어준다
    }
}

ISR(INT3_vect) {
    if (digitalRead(PhaseB)) cnt++;

    else cnt--;

    if (cnt >= 15) cnt = 0;

    else if (cnt <= 0) cnt = 0;
}

ISR(TIMER1_COMPA_vect) {
    volatile unsigned short index_count = count; //인덱스를 지역번수로 받아와서 실행속도를 높인다
    volatile unsigned char inter_check0 = check0; //0번 버퍼 사용중 여부를 지역변수로 받아온다
    volatile unsigned char inter_check1 = check1; //1번 버퍼 사용중 여부를 지역변수로 받아온다
    volatile char inter_mode = playing_mode; //재생모드 설정을 위한 변수를 지역변수로 받아온다

    switch (inter_mode) {
    case 0: //mono 22.05k 8bit
        if ((index_count + inter_check0) > 2999) { //0번버퍼 재생중에 버퍼 끝에 도달하면
            index_count = 0; //인덱스 초기화
            inter_check0 = 1; //0번버퍼 끝났음을 표시
        }

        if ((index_count + inter_check1) > 2999) {
            index_count = 0; //인덱스 초기화
            inter_check1 = 1; //1번버퍼 끝났음을 표시
        }
        OCR2B = (uint8_t)ptr[index_count];
        OCR2A = (uint8_t)ptr[index_count];
        OCR4B = (uint8_t)ptr[index_count];
        OCR4A = (uint8_t)ptr[index_count]; //모노 파일이므로 모든 정보가 동일하게 들어간다

        index_count += 1; //8비트이므로 인덱스는 1씩 증가
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1; //전역변수에 현재 상태를 반영
        break;

    case 1: //mono 22.05k 16bit
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

        index_count += 2; //모노 16비트이므로 인덱스는 2씩 증가
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    case 2: //mono 44.1k 8bit
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

        index_count += 1; //모노 8비트이므로 인덱스는 2씩 증가
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    case 3: //mono 44.1k 16bit
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

        index_count += 2; //모노 16비트이므로 인덱스는 2씩 증가
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    case 4: //stereo 22.05k 8bit
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

        index_count += 2; //스테레오 8비트이므로 인덱스는 2씩 증가
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    case 5: //stereo 22.05k 16bit
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

        index_count += 4; //스테레오 16비트이므로 인덱스는 4씩 증가
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    case 6: //stereo 44.1k 8bit
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

        index_count += 2; //스테레오 8비트이므로 인덱스는 2씩 증가
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    case 7: //stereo 44.1k 16bit
        if ((index_count + inter_check0) > 2996) {
            index_count = 0;
            inter_check0 = 1;
        }

        if ((index_count + inter_check1) > 2996) {
            index_count = 0;
            inter_check1 = 1;
        }
        OCR2B = (uint8_t)ptr[index_count];
        OCR2A = (uint8_t)ptr[index_count + 1] + 0x80; //16비트가 signed data로 나오기 때문에 이를 PWM에 넣어주기 위해 offset을 0x80만큼 준다
        OCR4B = (uint8_t)ptr[index_count + 2];
        OCR4A = (uint8_t)ptr[index_count + 3] + 0x80;

        index_count += 4; //스테레오 8비트이므로 인덱스는 2씩 증가
        count = index_count;
        check0 = inter_check0;
        check1 = inter_check1;

        break;

    default:
        break;
    }

   
}
