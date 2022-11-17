/*
File name : Wav_file_player
date : 2021.11.28
made : 기계공학과 12170410 장한빈
youtube link : https://youtu.be/8RXQRCF8Qzc

!!!!! 읽어주세요 !!!!! : 전류 부족한 현상이 발생하여 연주가 제대로 진행되지 않는 현상이 있었습니다. 이를 막고자 Arduino mega에 12V, 3A를 추가적으로 공급하였습니다.
                        SD파일에는 소리를 낮춘 WAV파일을 넣어주었으며 소리가 높은 WAV파일을 넣을 시 연주가 안되는 현상이 있습니다! 
                        소리 크기가 낮은 파일로 확인 부탁드립니다!!

abstract : Arduino Mega를 이용하여 SD파일의 WAV파일을 읽고 스피커로 노래를 트는 코드
           WAV 파일을 WAV format에 맞게 읽으며, 16bit,8bit / stereo,mono / 22050Hz,44100Hz인지 확인하며
           그에 맞게 interrupt 및 알고리즘을 바꿔주어 동작하도록 제작함.

mode는 sd파일에서 파일을 읽고 노래를 선택하는 부분과 sd파일에서 wav파일을 읽어 연주하는 부분으로 나뉘어진다.

노래를 선택하는 부분 : 노래를 선택하는 부분에는 encoder센서값을 받아 interrupt를 작동하게 하여 노래를 넘기고 또는 선택할 수 있도록 하였다.
wav 파일을 연주하는 부분 : wav파일을 읽어 format을 확인하고 그에 맞게 file을 읽고 연주한다.

Interrupt : 44.1kHz에 맞게 timer interrupt가 걸릴 수 있도록 CTC mode를 설정한다.
그보다 빠른 62.5kHz에 맞도록 PWM을 보내기 위해 Fast PWM을 설정한다.
Rotary encoder interrupt, switch interrupt는 센서값이 바뀔때 interrupt가 걸리도록 설정한다.

*/
#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
#include <stdlib.h>
#define chipSelect 53

// audio play + SD card
unsigned char buf1[3000];                                 // wav파일의 정보를 저장하는 buffer1
unsigned char buf2[3000];                                 // wav파일의 정보를 저장하는 buffer2
unsigned char* buf;                                       // wav파일의 정보를 play하는 buffer
unsigned char audio_format = 0;                           // 파일이 stereo 또는 mono임을 확인하는
unsigned char bit_divided[1][2];                          // format정보를 확인하기 위해 사용하는 변수
unsigned char sample_rate_divided[1][4];                  // format정보를 확인하기 위해 사용하는 변수
volatile int sample_rate;                                 // sample rate 정보를 담는 변수
volatile int bit_type;                                    // 8bit 혹은 16bit 임을 확인하는 변수
volatile unsigned int count = 0b0000000000000000;         // 현재 연주해야할 buf의 주소를 알려주는 변수 
volatile char next;                                       // buffer에 data가 모두 저장이 됐는지 또는 연주가 됐는지를 알려주는 변수


// save name list
char name_list[10][20];                                   // SD 카드에서 읽은 파일의 이름을 저장하는 변수

// encoder
volatile int cnt = 2;                                     // encoder의 회전정도를 저장하는 변수 
volatile int encoder_sw = 0;                              // encoder sw의 정보를 담는 변수

// LCD
LiquidCrystal lcd(12,11,5,4,3,2);                         // lcd on

// SD card
char *save_name;                                          // 임시적으로 sd카드의 이름을 저장하는 변수

// flag
char flag1 = 0;                                           // 모드가 변경될 때 1번만 돌면 되는 코드를 위한 flag 변수
char flag2 = 0;                                           // 모드가 변경될 때 1번만 돌면 되는 코드를 위한 flag 변수


File dataFile;                                            // dataFile에 wav파일 정보를 담습니다.

void setup() {
  Serial.begin(115200);                                   // serial on, sd카드를 못읽을 시 Boom!이라는 massage 출력
  SD.begin(53);                                           // sd begin

  save_name_list();                                       // 초기 setup시 파일의 이름을 name_list에 저장한다. (10개까지 저장하도록 변수를 설정해 놓음)

  // LCD
  lcd.begin(16,2);                                        // LCD begin
  lcd.setCursor(0,0);
}

void loop() {                                             // 지금 연주중인 buffer가 1번일 때 2번에 buffer 저장
  if (encoder_sw == 0)
  {
    if (flag1 == 0)                                       // choice모드 초기 설정
    {
      encoder_interrupt_init();                           // encoder interrupt관련 설정
      sei();
      flag1 = 1;                                          // 모드 변경시 한번만 도는 코드를 위한 flag                  
      flag2 = 0;                        
    }

    lcd.clear();                                          // LCD 설정
    lcd.setCursor(0,0);
    lcd.print(name_list[cnt]);                            // 초기에 저장한 name list에 encoder의 cnt에 따라 print한다
    lcd.setCursor(0,1);
    lcd.print("START");
    delay(500);
  }

  else
  {
    if (flag2 == 0)
    {
      playing_mode();                                     // wav format정보 추출, encoder interrupt disable        
      PWM_init();                                         // PWM관련 register설정
      timer_init();                                       // timer 관련 register설정
      sei();    
      flag1 = 0;
      flag2 = 1;
      lcd.setCursor(0,1);
      lcd.print("STOPPED");
    }
    
    dataFile.read(buf2, sizeof(buf2));                    // 버퍼2에 데이터를 저장한다. (연주는 buf1의 정보 이용)
    while (!(next));                                      // buf1연주가 끝날때 까지 기다린다. (연주가 끝나면 interrupt에서 next1로 설정)   
    buf = buf2;                                           // buf에 buf2의 주소를 넘겨준다.
    next &= 0x00;                                         // 다음 buf를 저장할때 기다리도록 next = 0


    dataFile.read(buf1, sizeof(buf1));                    // 버퍼1에 데이터를 저장한다.               
    while (!(next));                                      // 버퍼에 저장이 모두 끝나면 interrupt에서 데이터를 버퍼 data를 연주할때까지 기다린다. (연주가 끝나면 next가 01로 바뀐다. )
    buf = buf1;                                           // buf에 buf1의 주소를 넘겨준다.
    next &= 0x00;                                         // 다음 buf를 저장할때 기다리도록 next = 0
  }

  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//                                                     // 초기 설정 함수 //
void timer_init()
{
  TCCR1A &= ~_BV(WGM11); // CTC mode
  TCCR1A &= ~_BV(WGM10);
  TCCR1B |= _BV(WGM12);
  TCCR1B &= ~_BV(WGM13); 

  TCCR1B |= _BV(CS10); // No prescale
  TCCR1B &= ~_BV(CS11); 
  TCCR1B &= ~_BV(CS12); 

  if (sample_rate == 22050) // sample rate에 맞게 OCR1A를 설정
  {
    OCR1A = 724;
  }
  else
  {
    OCR1A = 362;
  }
  TCNT1 = 0x0000;                     // timer counter clear

  TIMSK1 |= _BV(OCIE1A);              // Timer interrupt mask register
  TIFR1 |= _BV(OCF1A);                // flag clearing
}

void PWM_init()                       // 출력 PWM 초기설정 TOP = 255 / OCRnA는 그 아래
{
  DDRB |= 0b00010000;                 //PB4 출력설정
  DDRH |= 0b01011000;                 //PH3,4,6 출력설정

  // OC2A,B의 Fast PWM설정 
  TCCR2A &= 0b00001100;               // WGM22 = 0, WGM21 = 1 , WGM20 = 1 : fast PWM 설정 // COM2A1 = 1, COM2A0 = 0 , COM2B1 = 1, COM2B0 = 0: non inverting mode
  TCCR2A |= 0b10100011;
  TCCR2B &= 0b11110000;               // CS22 = 0, CS21 = 0, CS20 = 1 : No prescaler
  TCCR2B |= 0b00000001;

  // OC4A,B의 Fast PWM설정
  TCCR4A &= 0b00001100;               // WGM43 = 0, WGM42 = 1, WGM41 = 0 , WGM40 = 1 : fast PWM 설정 // COM4A1 = 1, COM4A0 = 0, COM4B1 = 1, COM4B0 = 0 : non inverting mode
  TCCR4A |= 0b10100001;
  TCCR4B &= 0b11100000;               // CS42 = 0, CS41 = 0, CS40 = 1 : No prescaler
  TCCR4B |= 0b00001001;

  TCNT4 = 0; // TCNT4 clear
  TCNT2 = 0;
}

// Interrupt Service Routine
ISR (TIMER1_COMPA_vect)    // 44.1kHz 또는 22.05kHz 마다 PWM 값을 바꿔준다.
{
  if (!(next))
  {
    if (audio_format == 2)                                                            // stereo 일때
    {
      OCR2B = (char)(buf[count]);                                                     // OC2B는 OCR2B에 따라 PWM이 결정된다.
      OCR2A = (char)((buf[count + 0b0000000000000001] + 0x80)&0b1111111111111111);    // OC2A는 OCR2A에 따라 PWM이 결정된다.
      OCR4B = (char)(buf[count + 0b0000000000000010]);                                // OC4B는 OCR4B에 따라 PWM이 결정된다.
      OCR4A = (char)((buf[count + 0b0000000000000011] + 0x80)&0b1111111111111111);    // OC4A는 OCR4A에 따라 PWM이 결정된다.
      count = count + 0b0000000000000100;                                             // count는 한번 interrupt 마다 4개씩 커진다. count가 buf크기를 넘으면 1으로 초기화된다.
    
      if (count > 0b0000101110110111)                                                 // 읽는 data가 count수를 초과하면 count를 초기화하고 next333 flag를 1로 바꿔준다.
      {
        count &= 0b0000000000000000;
        next = 0x01;
      }
    }
    else                                                                            // mono 일때
    {
      OCR2B = (char)(buf[count]);                                                   // OC2B는 OCR2B에 따라 PWM이 결정된다.
      OCR2A = (char)((buf[count + 0b0000000000000001] + 0x80)&0b1111111111111111);  // OC2A는 OCR2A에 따라 PWM이 결정된다.
      OCR4B = (char)(buf[count]);                                                   // OC4B는 OCR4B에 따라 PWM이 결정된다.
      OCR4A = (char)((buf[count + 0b0000000000000001] + 0x80)&0b1111111111111111);  // OC4A는 OCR4A에 따라 PWM이 결정된다.
      count = count + 0b0000000000000010;

      if (count > 0b0000101110110111)                                               // 읽는 data가 count수를 초과하면 count를 초기화하고 next333 flag를 1로 바꿔준다.
      {
        count &= 0b0000000000000000;
        next = 0x01;
      }
    }
  }
    
}

ISR(INT3_vect) // encoder의 회전에 따라 cnt값을 증/감하는 ISR
{
  if((PIND & 0b00000100) == 0b00000100 ) cnt++; // interrupt가 걸렸을 때 On이면 cnt증가, Off면 cnt감소
  else
  {
    cnt--;
    if(cnt==1) cnt = 2;
  }
}

ISR(INT1_vect) // encoder sw에 interrupt가 걸릴 시 encoder_sw를 변경한다.
{
  if (encoder_sw == 0)
  {
    encoder_sw = 1;
  }
  else
  { 
    encoder_sw = 0;
  }
}

// function
void encoder_interrupt_init()
{
  SREG |= 0b10000000;
  DDRD &= 0b11110001; // PD1,2,3 (rotary encoder)를 입력으로 설정.
 
  EICRA &= ~_BV(ISC30); // encoder PHA를  falling edge를 설정
  EICRA |= _BV(ISC31);

  EICRA &= ~_BV(ISC10); // encoder switch를  falling edge를 설정
  EICRA |= _BV(ISC11);

  EIFR |= _BV(INTF3); // INTF3 clear
  EIMSK |= _BV(INT3); // enable external interrupt
  EIFR |= _BV(INTF1); // INTF1 clear
  EIMSK |= _BV(INT1); // enable external interrupt
}

void playing_mode()
{

  EIFR |= _BV(INTF3); // INTF3 clear
  EIMSK &= ~_BV(INT3); // disable external interrupt
  
  if(!SD.begin(chipSelect))
  {
    Serial.print("Bomm!!!");
    return;
  }
  
  dataFile = SD.open(name_list[cnt]);                            // wav 파일을 연다.               

  for (int i = 0; i < 12; i++){                            // RIFF부분은 받아올 정보 없이 Pass한다.
    dataFile.read();
  }
                                                           // File Format부분은 stereo or mono 그리고 44100Hz or 22050Hz의 정보를 빼낸다.
  for (int i = 12; i < 36; i++){
    if (i == 22)                                           // stereo이면 2를 mono이면 1을 audio_format에 저장한다. 
    {
      audio_format = dataFile.read();
    }
    else if (i == 24 || i == 25 || i == 26 || i == 27)     // Sample rate에 대한 정보를 담고있는 부분을 sample_rate_divided에 담는다
    {
      sample_rate_divided[1][i-23] = dataFile.read();
    }
    else if (i == 34 || i == 35)
    {
      bit_divided[1][i-33] = dataFile.read();              // 8bit파일인지 16bit파일인지를 담는 부분을 저장
    }
    else
    {
      dataFile.read();
    }
  }
                                                          // sample divided에 따라 44100Hz 인지 22050Hz 인지 확인한다
  if (sample_rate_divided[1][1] == 0x44 && sample_rate_divided[1][2] == 0xAC) 
  {
    sample_rate = 44100;                                 // sample rate : 44100 Hz
  }
  else if ((sample_rate_divided[1][1] == 0x22 && sample_rate_divided[1][2] == 0x56))
  {
    sample_rate = 22050;                                 // sample rate : 22050 Hz
  }

  if (bit_divided[1][1] == 0x10)
  {
    bit_type = 16;                                       // 16bit signed
  }
  else
  {
    bit_type = 8;                                        // 8bit signed
  }
                                                           
  for (int i = 36; i < 44; i++){                         // data부분 처음 8자리는 필요없으므로 단순읽기.
    dataFile.read();
  }

  dataFile.read(buf1, sizeof(buf1));                     // 처음 시작시 버퍼1에 data를 저장헤둔다.

  buf = buf1;
  next = 0x00;                                           // 다음 buf를 저장할때 기다리도록 next = 0 
}

void save_name_list()
{
  File root;
  root = SD.open("/"); // open(filepath) : 즉 file 경로를 입력인자로
  
  int i = 1;
  int cnt = 10;        // 파일이 10개 이하라고 가정하고 cnt를 잡음. 10개의 파일까지 name_list에 저장한다.
  char *save_name;


  while (true)
  {
    File entry = root.openNextFile(); // 다음 file 또는 directory를 report한다.
    save_name = entry.name();
    strcpy(name_list[i],save_name);
    
    if (i == cnt)                     // 10번을 다 일겄으면 while문을 빠져나온다
    {
      break;
    }
    entry.close();
    i++;
  }
}