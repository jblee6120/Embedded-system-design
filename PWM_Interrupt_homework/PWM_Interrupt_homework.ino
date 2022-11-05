/*-------------------------------------------------------------------------------------------------------
file name : PWM_Interrupt_homework

made : 12170410 기계공학과 장한빈

data : 2021.11.02

youtube link : https://youtu.be/-FRdAowtVuU

abstract : PIN11(PB5,OC1A) - Fast PWM generate
           PIN18(PD3,INT3) - external register
           핀11번에서 Fast PWM mode로 설정하고 1000Hz의 속도로 PWM신호를 보낸다.
           Serial에서 duty값을 받아 해당하는 PWM신호를 external interrupt service를 이용하여 측정한다.
           이후 입력값과 측정값을 다시 serial print한다.            
-------------------------------------------------------------------------------------------------------*/

volatile float time_start;
volatile float time_end;
volatile float time_gap;
volatile float measured_duty;
int duty = 50;
int buffer;

void PWM_init() // PWM 초기설정
{
  DDRB |= 0b00100000;              // 출력설정

  // mode : fast PWM mode, when TCNT1 is matched at OCR1A OC1A clearing.
  TCCR1A &= 0b00111100; 
  TCCR1A |= 0b10000010; // WGM11 = 1 , WGM10 = 0 // COM1A1 = 1 , COM1A0 = 0
  TCCR1B &= 0b11100111; 
  TCCR1B |= 0b00011000; // WGM13 = 1, WGM12 = 1
  

  ICR1 = (uint16_t)1999; // TOP값 설정
  OCR1A = (uint16_t)(1999.0*(float)duty*0.01); // TCNT1이 OCR1A까지 증가함.


  // prescale clk/8으로 설정 
  TCCR1B &= ~_BV(CS12);
  TCCR1B |= _BV(CS11);
  TCCR1B &= ~_BV(CS10);

  // counter 0값으로 초기화
  TCNT1 = 0x0000;
}

void interrupt_init() // interrupt관련 초기 설정
{
  EICRA &= ~_BV(ISC31); 
  EICRA |= _BV(ISC30); // any edge detection
  EIFR |= _BV(INTF3); // flag clear
  EIMSK |= _BV(INT3); // external interrupt enable 
}

ISR(INT3_vect)
{
  if (PIND & 0b00001000) // interrupt가 걸렸을때 입력 상태를 확인. ON인 상태면 rise edge, OFF 상태이면 falling edge임을 알 수 있음
  {
    time_start = TCNT1;
  }
  else // falling edge
  {
    time_end = TCNT1;
    time_gap = time_end - time_start;
    measured_duty = time_gap/2000.0*100; // 측정된 duty값
  }
}

void setup() {
  Serial.begin(115200);
  PWM_init(); // PWM 초기 설정
  interrupt_init(); // interrupt 초기설정
  sei(); // global interrupt enable
}

void loop() {
  if (Serial.available())
  {
    buffer = Serial.parseInt(); // 입력값을 buffer에 저장함.
    if (buffer != 0)  // 입력값에 값에 대입 이후 0으로 입력되는 오류가 있어서 넣었습니다.
    {
      if (buffer < 5) // buffer가 5보다 작으면 duty값을 5로
      {
        buffer = 5;
        duty = buffer;
        OCR1A = (uint16_t)(1999.0*(float)duty*0.01);

      }
      else if (buffer > 95) // buffer가 95보다 작으면 duty값을 95로
      {
        buffer = 95;
        duty = buffer;
        OCR1A = (uint16_t)(1999.0*(float)duty*0.01);
      }
      else{ // 입력한 duty값으로 설정
        duty = buffer;
        OCR1A = (uint16_t)(1999.0*(float)duty*0.01);
      }
    }
  }
  
  Serial.print(duty);
  Serial.print(" ");
  Serial.println(measured_duty);

  delay(20);
}
