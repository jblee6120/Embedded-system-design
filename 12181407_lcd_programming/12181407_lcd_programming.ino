//12181407 이종범 임베디드시스템 설계 과제2
//YouTube Link: https://youtu.be/kWopeNGt-vw
//File name: 12181407_lcd_programming.ino
//<설명>
//이 프로그램은 ADC와 LCD를 이용해서 0~5V의 전압과 전압레벨에 맞는 문구를 LCD화면에 출력하고
//각 단계별로 LED를 켜주는 프로그램이다.
//
//동작은 다음과 같다
//1. Potentiometer로 0~5V의 전압을 아두이노로 보낸다
//2. Arduino의 ADC로 0~5V의 전압을 0~1023의 디지털 값으로 변환해준다
//3. 0~1023의 디지털 값을 0~5인 전압값으로 다시 변환해준다
//4. LCD화면에 변환해준 전압값을 출력해준다
//5. 전압을 0~7단계까지 총 8단계로 나눠준다. 이 때, ADC로 되어있는 0~1023을 8개 구간으로 나눠주면 된다
//6. 각 구간에 맞는 문구를 LCD에 출력해준다.
//7. 각 구간에 맞는 LED를 켜준다.
//
//<Arduino MEGA 2560 - Peripheral Board간의 연결>
//		Arduiono MEGA 2560		Peripheral Board
//			A0(PF0)					LED1
//			A1(PF1)					LED2
//			A2(PF2)					LED3
//			A3(PF3)					LED4
//			A4(PF4)					LED5
//			A5(PF5)					LED6
//			A6(PF6)					LED7
//			A7(PF7)					LED8
//			
//			Potentiometer 연결
//		Arduiono MEGA 2560		Peripheral Board
//			A8(PK0)					ANA1
//
//			LCD 연결
//		Arduiono MEGA 2560		Peripheral Board
//			12						Rs
//			11						E
//			5						D4
//			4						D5
//			3						D6
//			2						D7

// <각 단계별 LED출력 및 LCD Message>
// 0단계: LED 1개 ON, Low Voltage 출력
// 1단계: LED 2개 ON, Low Voltage 출력
//2단계: LED 3개 ON, Low Voltage 출력
//3단계: LED 4개 ON, Good Voltage 출력
//4단계: LED 5개 ON, Good Voltage 출력
//5단계: LED 6개 ON, Good Voltage 출력
//6단계: LED 7개 ON, Good Voltage 출력
//7단계: LED 8개 ON, Danger Voltage 출력

#include <LiquidCrystal.h> //LCD 라이브러리 사용을위한 헤더파일 호출
#define RANGE 1023 // 0~1023의 범위를 0~1로 바꾸기 위한 전처리기 선언
LiquidCrystal display(12, 11, 5, 4, 3, 2); // LCD 객체인 display선언 Rs, E, D4~D7까지 사용하는 4선 방식을 사용함
int potentio; // Potentiometer에서 나오는 전압값을 ADC변환해서 저장할 변수 선언
int level; // 0~7단계의 전압 단계를 나타내는 변수
void setup(){
	DDRF |= 0XFF; //PF0~PF7을 OUTPUT으로 설정
	display.begin(16, 2); //LCD를 2행 16열로 쓰겠다고 설정
}

void loop(){
	display.setCursor(0, 0); //0번째 줄 0번자리에 커서를 위치한다
	display.print("Voltage="); //Voltage= 라는 문자열을 출력한다
	potentio = analogRead(A8); //ADC값을 변수에 저장
	display.setCursor(9, 0); //0~5V의 전압값을 출력하기 위해 커서 이동
	display.print((float)potentio / RANGE * 5.0); //전압값 변환: potentio(0~1023) -> 0~5V로 LCD에 출력
	display.setCursor(0, 1); //1번 줄의 0번째 칸으로 커서 이동
	level = potentio / 128; //전압을 8단계로 나누기 위해 1023까지의 값을 갖는 potentio를 128로 나눠준다
	
	

	switch (level) { // 각 전압단계별 LED출력과 그에따른 LCD출력
	case 0: // 0단계: ADC 0~127까지
		PORTF = 0x01; // LED는 1개만 ON
		display.print("Low voltage"); // LCD에는 LOW Voltage 출력
		break;
	case 1: // 1단계: ADC 128~255까지
		PORTF = 0x03; // LED는 2개만 ON
		display.print("Low voltage"); // LCD에는 LOW Voltage 출력
		break;
	case 2: // 2단계: ADC 256~383까지
		PORTF = 0x07; // LED는 3개만 ON
		display.print("Low Voltage"); // LCD에는 LOW Voltage 출력
		break;

	case 3: // 3단계: ADC 384~511까지
		PORTF = 0x0F; // LED는 4개 ON
		display.print("Good Voltage"); // LCD에는 Good Voltage 출력
		break;

	case 4: // 4단계: ADC 512~639까지
		PORTF = 0x1F; // LED는 5개 ON
		display.print("Good Voltage"); // LCD에는 Good Voltage 출력
		break;

	case 5: // 5단계: ADC 640~767까지
		PORTF = 0x3F; //LED는 6개 ON
		display.print("Good Voltage"); // LCD에는 Good Voltage 출력
		break;

	case 6: // 6단계: ADC 768~894
		PORTF = 0x7F; // LED는 7개 ON
		display.print("Good Voltage"); // LCD에는 Good Voltage 출력
		break;

	case 7: // 7단계: ADC 895~1023
		PORTF = 0xFF; // LED를 8개 전부 ON, 이때 LED8은 buzzer완 연결되어 소리가 난다.
		display.print("Danger Voltage"); // 7단계이므로 LCD에는 Good Voltage 출력
		break;

	default:
		break;
	}

	delay(100); // LCD 출력결과를 표시하기 위해 100ms동안 대기
	display.clear(); // 새로운 결과를 출력하기 위해 LCD를 초기화시킨다.
}