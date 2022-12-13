//임베디드 시스템 설계 Line Tracer Project
//File name: 
//maker: 12151179 이윤호, 12181407 이종범
//YouTube Link: 
//Abstract
// 이 프로그램은 2개의 라인을 인식해 라인의 가운데를 따라 주행하는 Line Tracer 프로그램이다. 프로그램의 동작원리는 다음과 같다.
// 1. 라인스캔카메라로 2개의 라인을 검출한다
// 2. 2개의 라인 위치를 구한 뒤, 두 위치값의 평균을 취해 두 라인의 중앙의 위치를 결정한다.(라인트레이서가 현재 보고있는 위치)
// 3. 기준 위치와 현재 위치의 오차를 구한 뒤, 비례제어를 이용해 모터의 회전속도를 결정한다.
// 4. 결정된 모터의 회전속도에 맞는 주파수를 구한다.
// 5. 결정된 주파수를 만드는 DDA A, DDA M 값을 결정한다.
// 6. Software counter를 이용해 DDA 알고리즘을 구현한 뒤, 스텝모터에 펄스를 인가한다.
// 
 // 결선
 // Machine          Arduino
 // EN               17(PH0)
 // STEP1            18(PD0)
 // DIR1             19(PD1)
 // STEP2            20(PD2)
 // DIR2             21(PD3)
 //
 // SI               23(PA1)
 // CLK              22(PA0)
 // AO               A0(PF0, ADC0)
 //

#define CLK PA0 //라인스캔센서의 CLK신호를 출력하는 핀을 전처리했음
#define SI PA1 //센서에서 아날로그 신호를 출력 준비시키는 SI 핀을 전처리했음
#define carrier_freq 20000 //DDA 알고리즘에서 carrier_freqency를 20kHz로 설정한다
# define PI 3.141592654 //Low Pass Filter를 사용할 때 쓰이는 PI값을 정의

long difference = 1000; //스텝모터의 회전속도 즉, 펄스 수의 급격한 변화로 발생하는 탈조를 막기위한 조속기 알고리즘의
                        //속도변화의 최댓값

volatile unsigned int camera_data[140] = { 0 }; //센서의 ADC 값을 받아오는 배열, LPF를 거치지 않은 RAW data를 담는 배열이다

void ADC_Init(); //ADC 초기화 함수

float ts = 0.01; //반복적인 연산을 위한 샘플링 타임을 0.01로 설정
uint32_t MicrosSampleTime; //마이크로단위로 변경한 샘플링타임을 담는 변수
uint32_t time_out; //주기적연산이 끝나는 시간을 담는 변수

int left_falling = -1; //왼쪽에서 검출된 라인의 위치를 저장하는 변수
int right_falling = -1; //오른쪽에서 검출된 라인의 위치를 저장하는 변수
int current_position; //라인트레이서가 현재 바라보는 위치를 저장하는 변수

volatile unsigned int Left_cnt1 = 0; //Software Counter로 DDA 알고리즘 구현을 위한 변수선언
volatile unsigned int Left_cnt2 = 0;
volatile unsigned int Right_cnt1 = 0;
volatile unsigned int Right_cnt2 = 0;

long temp_Left_DDA_A=2000;//계산결과로 나온 왼쪽 바퀴의 회전속도에 해당하는 DDA 알고리즘의 A값
long Left_DDA_A = 2000; //실제 스텝모터에 인가되는 왼쪽 바퀴 DDA 알고리즘의 A값
int Left_DDA_M = (int)(Left_DDA_A / 100) >> 1; //왼쪽 바퀴에 인가되는 펄스의 지속시간을 결정하는 DDA M변수

long temp_Right_DDA_A=2000;//계산결과로 나온 오른쪽 바퀴의 회전속도에 해당하는 DDA 알고리즘의 A값
long Right_DDA_A = 2000;//실제 스텝모터에 인가되는 오른쪽 바퀴 DDA 알고리즘의 A값
int Right_DDA_M = (int)(Right_DDA_A / 100) >> 1;//오른쪽 바퀴에 인가되는 펄스의 지속시간을 결정하는 DDA M변수

int vel = 3000; //자동차의 직선속도 변수
int rotation_vel; //자동차의 회전속도 변수
int Kp = 55; //비례제어를 위한 Kp 변수

void setup() {
    ADC_Init(); //ADC 초기화
    Motor_Init(); //모터 구동을위한 초기화
    Timer_Init(); //Timer Interrupt 사용을 위한 초기화

    DDRA |= (1 << CLK) | (1 << SI); // CLK, SI 핀 OUTPUT 설정

    MicrosSampleTime = (uint32_t)(ts * 1e6);
    time_out = micros() + MicrosSampleTime;
}

void loop()
{
    int x = 64; //센서에서 나오는 데이터의 수가 0~127이므로 중앙값인 64를 기준으로 로봇이 가운데로 정렬한다
    left_falling = -1; //왼쪽 라인의 위치를 -1로 초기화, -1은 라인 미검출 시 대입한다.
    right_falling = -1; //오른쪽 라인의 위치를 -1로 초기화, -1은 라인 미검출 시 대입한다.

    PORTA |= (1 << SI); //SI를 HIGH로 변경해 센서에서 아날로그 신호를 출력하기 위한 준비를 시킨다.
    PORTA |= (1 << CLK); //CLK를 HIGH로 변경한다
    PORTA &= ~(1 << SI); //SI를 LOW로 변경한다

    delay(1);

    for (int i = 0; i < 129; i++) {
        ADC_get(i); //i번째 센서의 아날로그 값을 얻는 함수 실행
        PORTA &= ~(1 << CLK); //다음 센서의 신호를 받기 위해 CLK신호를 LOW로 만든다
        PORTA |= (1 << CLK); //CLK 신호를 HIGH로 변경해 다음 센서가 아날로그 신호를 출력하도록 한다.
    }
    PORTA &= ~(1 << CLK); //모든 센서의 신호를 받으면 CLK를 LOW로 만들어야 다음 세트에 해당하는 센서들의 신호를 받을 수 있다

    while (x >= 0) { //라인스캔센서의 중앙인 64에서부터 왼쪽 끝까지 모든 데이터를 읽어본다 
        if (camera_data[x] < 300) { //ADC값이 기준치인 300미만일 때
            left_falling = x; //해당 인덱스가 왼쪽에서 검출되는 라인의 위치이므로 대입해준다
            x = 64; //오른쪽 라인 검출을 위해 인덱스를 중앙으로 초기화
            break; //라인 검출 시 바로 while문을 빠져나온다.
        }
        x -= 1; //64에서 0으로 가는 방향이 자동차를 기준으로 했을 때, 왼쪽이므로 인덱스를 감소시킨다.
    }

    while (x <= 127) { //라인스캔센서의 중앙인 64에서부터 왼쪽 끝까지 모든 데이터를 읽어본다
        if (camera_data[x] < 300) {//ADC값이 기준치인 300미만일 때
            right_falling = x; //해당 인덱스가 왼쪽에서 검출되는 라인의 위치이므로 대입해준다
            x = 64; //왼쪽 라인 검출을 위해 인덱스를 중앙으로 초기화
            break; //라인 검출 시 바로 while문을 빠져나온다.
        }
        x += 1; //64에서 127로 가는 방향이 자동차를 기준으로 했을 때, 오른쪽이므로 인덱스를 증가시킨다.
    }
    switch (left_falling) //왼쪽 라인의 위치에 대하여
    {
    case -1: //미검출 시
        left_falling = right_falling - 110; //오른쪽 라인의 위치를 기준으로 왼쪽으로 110만큼의 위치로 왼쪽라인의 위치를 임의로 정해준다.
        vel = 2600; //한 쪽의 라인만 검출된 것은 곡선구간이라는 뜻이므로 감속주행을 위해 직선속도값을 줄여준다.
        break;

    default:
        vel = 3900; //왼쪽 라인 검출 시, 직선으로 판단하고 직선속도값을 증가시킨다
        break;
    }
    switch (right_falling) //왼쪽 라인의 위치에 대하여
    {
    case -1: //미검출 시
        right_falling = left_falling + 110; //오른쪽 라인의 위치를 기준으로 왼쪽으로 110만큼의 위치로 왼쪽라인의 위치를 임의로 정해준다.
        vel = 2600; //한 쪽의 라인만 검출된 것은 곡선구간이라는 뜻이므로 감속주행을 위해 직선속도값을 줄여준다.
        break;

    default:
        vel = 3900; //왼쪽 라인 검출 시, 직선으로 판단하고 직선속도값을 증가시킨다
        break;
    }
    current_position = (int)(left_falling + right_falling) / 2; //현재 로봇이 바라보는 방향은 왼쪽 선과 오른쪽 선의 중앙이므로 두 값의 평균을 취해준다.

    rotation_vel = (64 - current_position) * Kp; //모터의 회전속도는 위치값의 오차에 비례해서 설정해준다.(사실상의 제어값)

    temp_Left_DDA_A = round((2000000) / (vel - rotation_vel)); //회전속도에 맞는 펄스를 인가하기 위한 DDA 알고리즘의 A 값을 계산해준다
    temp_Right_DDA_A = round((2000000) / (vel + rotation_vel));
    
    //탈조현상을 방지하기 위한 조속기 모듈
    if(Left_DDA_A - temp_Left_DDA_A > difference) //현재 회전속도에 해당하는 DDA A값이 계산으로 반영되어야 할 DDA A값보다 1000이상 차이나는 경우(급격한 감속이 이루어지는 경우)
    {
      Left_DDA_A = Left_DDA_A - difference; //회전속도를 바로 바꾸지 않고 천천히 계산값에 수렴하도록 만든다
    }
    else if(Left_DDA_A - temp_Left_DDA_A < -difference) //현재 회전속도에 해당하는 DDA A값이 계산으로 반영되어야 할 DDA A값보다 1000이상 차이나는 경우(급격한 가속이 이루어지는 경우)
    {
      Left_DDA_A = Left_DDA_A + difference; //회전속도를 바로 바꾸지 않고 천천히 계산값에 수렴하도록 만든다
    }
    else
    {
      Left_DDA_A = temp_Left_DDA_A; //모터의 현재 회전속도가 급격하게 변화하지 않는다면, 계산값을 바로 구동부에 적용시킨다.
    }

    if(Right_DDA_A - temp_Right_DDA_A > difference)
    {
      Right_DDA_A = Right_DDA_A - difference;
    }
    else if(Right_DDA_A - temp_Right_DDA_A < -difference)
    {
      Right_DDA_A = Right_DDA_A + difference;
    }
    else
    {
      Right_DDA_A = temp_Right_DDA_A;
    }
    
    Left_DDA_M = (int)(Left_DDA_A / 100) >> 1; //모터에 인가되는 펄스의 지속시간을 결정하는 DDA M 변수를 결정한다
    Right_DDA_M = (int)(Right_DDA_A / 100) >> 1;

    while (!((time_out - micros()) & 0x80000000)); //0.01초가 될 때까지 대기한다(반복적 연산 수행)
    time_out += MicrosSampleTime;
}

void ADC_Init() {
    ADMUX &= ~_BV(REFS1); 
    ADMUX |= _BV(REFS0);
    ADMUX &= ~_BV(ADLAR); //ADC 데이터를 오른쪽 정렬방식으로 사용한다는 설정

    ADCSRA |= _BV(ADEN); //ADC Enable
    ADCSRA &= ~_BV(ADSC); //start conversion = 0으로
    ADCSRA &= ~_BV(ADATE); //single conversion으로 만들어 사용자가 원할 때 conversion이 이루어지게 한다.
    ADCSRA |= _BV(ADIF); //ADIF 초기화
    ADCSRA &= ~_BV(ADIE); //ADC interrupt를 disable
    ADCSRA |= _BV(ADPS2); //ADC divisoin factor = 32로 설정 -> 500kHz clock 신호가 발생한다.
    ADCSRA &= ~_BV(ADPS1);
    ADCSRA |= _BV(ADPS0);
}

void ADC_get(unsigned char num) {
    ADCSRA |= (1 << ADSC); //ADC 변환을 시작한다.

    while (ADCSRA & (1 << ADSC)); //변환이 종료될 때 까지 기다린다.
    camera_data[num] = ADCW; //변환이 완료되면 ADCW에 있는 ADC값을 camera_data에 저장한다.
}

void Motor_Init()
{
    DDRH |= _BV(PH0);
    DDRD |= _BV(PD0);
    DDRD |= _BV(PD1);
    DDRD |= _BV(PD2);
    DDRD |= _BV(PD3);

    PORTH &= ~_BV(PH0); // Motor Driver 작동 시작
    PORTD &= ~_BV(PD1); // Motor Driver Direction 조정 0 : CW , 1 : CCW
    PORTD |= _BV(PD3);  // Motor Driver Direction 조정 0 : CW , 1 : CCW
    PORTD &= ~_BV(PD0); // step1의 시작값을 0으로 설정
    PORTD &= ~_BV(PD2); // step2의 시작값을 0으로 설정
}

void Timer_Init()
{
    TCCR1B &= ~_BV(WGM13);
    TCCR1B |= _BV(WGM12);
    TCCR1A &= ~_BV(WGM11);
    TCCR1A &= ~_BV(WGM10);  // CTC mode

    TCCR1B &= ~_BV(CS12);
    TCCR1B |= _BV(CS11);
    TCCR1B &= ~_BV(CS10); // set timer1 prescaler 8
    OCR1A = 99; // OCR1A at 20000kHz Carriar freq
    TCNT1 = 0;

    TIMSK1 |= _BV(OCIE1A); //Timer Interrupt Enable
    TIFR1 |= _BV(OCF1A); //Timer interrupt flag 초기화

    sei(); //global interrupt enable
}

ISR(TIMER1_COMPA_vect) //Interrupt Service Routine으로 DDA 알고리즘 구현
{
    Left_cnt1++; //cnt1을 1씩 증가시킨다
    if (Left_cnt1 > Left_DDA_M) //cnt1이 DDA M을 넘아가게 되면
    {
        PORTD &= ~_BV(PD2); //스텝모터에 인가되는 펄스는 LOW신호가 된다(펄스의 지속시간 결정)
    }

    Left_cnt2 += 100; //cnt2는 100씩 증가한다

    if (Left_cnt2 >= Left_DDA_A) //cnt2가 DDA A를 넘어가게 되면
    {
        PORTD |= _BV(PD2); //스텝모터에 High 신호를 인가하고
        Left_cnt2 -= Left_DDA_A; //cnt2는 다음 사이클에 맞는 값으로 돌아간다. cnt2의 최댓값은 DDA A ex) DDA A가 10일 때, cnt2는 3씩 증가하고 cnt2= 9->12면, 9->12->10
        Left_cnt1 = 0; //software counter 초기화
    }

    Right_cnt1++; //cnt1는 100씩 증가한다
    if (Right_cnt1 > Right_DDA_M) //DDA A가 DDAB 넘어가게 되면
    {
        PORTD &= ~_BV(PD0); //스텝모터에 인가되는 펄스는 LOW신호가 된다(펄스의 지속시간 결정)
    }

    Right_cnt2 += 100;// cnt2는 100씩 증가한다

    if (Right_cnt2 >= Right_DDA_A) //cnt2가 DDA A를 넘어가게 되면
    {
        PORTD |= _BV(PD0); //스텝모터에 High 신호를 인가하고
        Right_cnt2 -= Right_DDA_A; //cnt2는 다음 사이클에 맞는 값으로 돌아간다. cnt2의 최댓값은 DDA A ex) DDA A가 10일 때, cnt2는 3씩 증가하고 cnt2= 9->12면, 9->12->10
        Right_cnt1 = 0; //software counter 초기화
    }
}