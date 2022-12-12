/*
 * 결선
 * machine          arduino
 * EN               17(PH0)
 * STEP1            18(PD0)
 * DIR1             19(PD1)
 * STEP2            20(PD2)
 * DIR2             21(PD3)
 *
 * SI               23(PA1)
 * CLK              22(PA0)
 * AO               A0(PF0, ADC0)
 */

#define CLK PA0
#define SI PA1
#define carrier_freq 20000
# define PI 3.141592654

long difference = 1000;

volatile unsigned int camera_data[140] = { 0 };

//
volatile unsigned int filter_data[140] = { 0 };

class LPF
{
public:
    LPF(float Fc, float Ts);
    ~LPF();
    void calc();
public:
    float k1, k2;
    float xk, yk, uk;
};
LPF* LPF1;
//


void ADC_Init();

float ts = 0.01;
uint32_t MicrosSampleTime;
uint32_t time_out;

int left_falling = -1;
int right_falling = -1;
int current_position;

/**/
volatile unsigned int Left_cnt1 = 0;
volatile unsigned int Left_cnt2 = 0;
volatile unsigned int Right_cnt1 = 0;
volatile unsigned int Right_cnt2 = 0;

long temp_Left_DDA_A=2000;
long Left_DDA_A = 2000;
int Left_DDA_M = (int)(Left_DDA_A / 100) >> 1;

long temp_Right_DDA_A=2000;
long Right_DDA_A = 2000;
int Right_DDA_M = (int)(Right_DDA_A / 100) >> 1;

int vel = 3000;
int rotation_vel;
int Kp = 55;

void setup() {
    Serial.begin(115200);

    //
    LPF1 = new LPF(3.0, ts);
    //


    ADC_Init();
    Motor_Init();
    Timer_Init();

    DDRA |= (1 << CLK) | (1 << SI); // CLK, SI 핀 OUTPUT 설정

    MicrosSampleTime = (uint32_t)(ts * 1e6);
    time_out = micros() + MicrosSampleTime;
}

void loop()
{
    int x = 64;
    left_falling = -1;
    right_falling = -1;

    PORTA |= (1 << SI);
    PORTA |= (1 << CLK);
    PORTA &= ~(1 << SI);

    delay(1);

    for (int i = 0; i < 129; i++) {
        ADC_get(i);
        PORTA &= ~(1 << CLK);
        PORTA |= (1 << CLK);
    }
    PORTA &= ~(1 << CLK);

    while (x >= 0) {
        if (camera_data[x] < 300) {
            left_falling = x;
            x = 64;
            break;
        }
        x -= 1;
    }

    while (x <= 127) {
        if (camera_data[x] < 300) {
            right_falling = x;
            x = 64;
            break;
        }
        x += 1;
    }
    switch (left_falling)
    {
    case -1:
        left_falling = right_falling - 110;
        vel = 2600;
        break;

    default:
        vel = 3900;
        break;
    }
    switch (right_falling)
    {
    case -1:
        right_falling = left_falling + 110;
        vel = 2600;
        break;

    default:
        vel = 3900;
        break;
    }
    current_position = (int)(left_falling + right_falling) / 2;

    rotation_vel = (64 - current_position) * Kp;

    temp_Left_DDA_A = round((2000000) / (vel - rotation_vel));
    temp_Right_DDA_A = round((2000000) / (vel + rotation_vel));
    
    if(Left_DDA_A - temp_Left_DDA_A > difference)
    {
      Left_DDA_A = Left_DDA_A - difference;
    }
    else if(Left_DDA_A - temp_Left_DDA_A < -difference)
    {
      Left_DDA_A = Left_DDA_A + difference;
    }
    else
    {
      Left_DDA_A = temp_Left_DDA_A;
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
    
    Left_DDA_M = (int)(Left_DDA_A / 100) >> 1;
    Right_DDA_M = (int)(Right_DDA_A / 100) >> 1;

    while (!((time_out - micros()) & 0x80000000));
    time_out += MicrosSampleTime;
}

void ADC_Init() {
    ADMUX &= ~_BV(REFS1);
    ADMUX |= _BV(REFS0);
    ADMUX &= ~_BV(ADLAR);

    ADCSRA |= _BV(ADEN);
    ADCSRA &= ~_BV(ADSC);
    ADCSRA &= ~_BV(ADATE);
    ADCSRA |= _BV(ADIF);
    ADCSRA &= ~_BV(ADIE);
    ADCSRA |= _BV(ADPS2);
    ADCSRA &= ~_BV(ADPS1);
    ADCSRA |= _BV(ADPS0);
}

void ADC_get(unsigned char num) {
    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC));
    camera_data[num] = ADCW;
}

void Motor_Init()
{
    DDRH |= _BV(PH0);
    DDRD |= _BV(PD0);
    DDRD |= _BV(PD1);
    DDRD |= _BV(PD2);
    DDRD |= _BV(PD3);

    PORTH &= ~_BV(PH0); // Motor Driver 작동 시작
    PORTD &= ~_BV(PD1);  // Motor Driver Direction 조정 0 : CW , 1 : CCW
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

    TIMSK1 |= _BV(OCIE1A);
    TIFR1 |= _BV(OCF1A);

    sei();
}

ISR(TIMER1_COMPA_vect)
{
    Left_cnt1++;
    if (Left_cnt1 > Left_DDA_M)
    {
        PORTD &= ~_BV(PD2);
    }
    Left_cnt2 += 100;
    if (Left_cnt2 >= Left_DDA_A)
    {
        PORTD |= _BV(PD2);
        Left_cnt2 -= Left_DDA_A;
        Left_cnt1 = 0;
    }

    Right_cnt1++;
    if (Right_cnt1 > Right_DDA_M)
    {
        PORTD &= ~_BV(PD0);
    }
    Right_cnt2 += 100;
    if (Right_cnt2 >= Right_DDA_A)
    {
        PORTD |= _BV(PD0);
        Right_cnt2 -= Right_DDA_A;
        Right_cnt1 = 0;
    }
}

//
LPF::LPF(float Fc, float Ts)
{
    float Wc = 2.0 * PI * Fc;
    k1 = Wc * Ts / (Wc * Ts + 2.0);
    k2 = (Wc * Ts - 2.0) / (Wc * Ts + 2.0);
    xk = 0.0;
}

void LPF::calc()
{
    yk = xk + k1 * uk;
    xk = -k2 * xk + k1 * (1.0 - k2) * uk;
}

LPF::~LPF()
{
}
//