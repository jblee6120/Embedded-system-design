#include <LiquidCrystal.h>
#define PhaseA 18
#define PhaseB 19

LiquidCrystal display(12, 11, 5, 4, 3, 2);

char title[4][5] = { "embe", "dded", "syst", "em!!"}; //제목을 담고있는 배열, 기능 통합 시, SD카드에서 나온 제목을 담아야 함. 제목의 규격은 8.3으로 한다
char state[2][8] = { "Stopped", "Playing" }; //재생상태 문자열을 담고있는 배열

volatile int cnt = 0;
volatile int play_state = 0;
void ext_int3_init();
void ext_int1_init();

uint32_t MicrosSampleTime; // micro 단위로 변환한 sample_time을 저장하는 변수
uint32_t start_time; //주기적 연산을 시작하는 시간을 저장하는 변수
float sample_time; //주기적 연산시간을 sec 단위로 저장하는 변수


void setup() {
	/*pinMode(PhaseA, INPUT);
	pinMode(PhaseB, INPUT);
	pinMode(20, INPUT);*/
	DDRD &= ~0x0E; //PD3, PD2, PD1 을 INPUT으로 설정

	display.begin(16, 2); //2x16 display 설정
	
	sample_time = 0.1; //0.02초마다 주기적 연산을 한다.
	MicrosSampleTime = (uint32_t)(sample_time * 1e6); //0.02 sec를 micro단위로 변환


	ext_int3_init(); //INT3 interrupt 초기화
	ext_int1_init(); //INT1 interrupt 초기화
	sei(); //golbal interrupt enable 시작

	Serial.begin(115200); //기타 자질구레한 출력값 확인용 시리얼 통신 시작
	start_time = micros() + MicrosSampleTime; //주기적연산 시작시간 초기화

}

void loop() {
	display.setCursor(0, 0); //0번줄 0번째 칸에 커서 위치
	display.print("                "); //0번줄 전체 클리어
	display.setCursor(0, 0); //0번줄 0번째 칸에 커서 위치
	display.print(title[cnt]); // 노래 제목 출력 구현 테스트, 나중에 제목 출력하는 함수로 다 묶어버리기

	display.setCursor(0, 1); //1번줄 0번째 칸에 커서 위치
	display.print("                "); //1번줄 전체 클리어
	display.setCursor(0, 1); //0번줄 0번째 칸에 커서 위치
	display.print(state[play_state]); //재생상태 출력 구현 테스트, 나중에 재생상태 표시하는 함수로 다 묶어버리기

	//Serial.println(cnt); //자질구레한 출력 확인용 시리얼통신 출력
	//Serial.print(" ");
	//Serial.print(digitalRead(PhaseA));
	//Serial.print(" ");
	//Serial.println(digitalRead(PhaseB));

	//delay(1000); //display 유지를 위한 50ms동안의 delay.

	while (!((start_time - micros()) & 0x80000000)); //주기적 연산시간동안 대기
	start_time += MicrosSampleTime; //새로운 주기적연산 시작시간 갱신

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

	if (cnt > 3) cnt = 0;

	else if (cnt <= 0) cnt = 0;
}