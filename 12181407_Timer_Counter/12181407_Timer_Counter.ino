//12181407 이종범 Timer/Counter과제
//Youtube Link: https://youtu.be/r4uiZR-4LzU
//File name: 12181407_Timer_Counter.ino
//
//<설명>
//이 프로그램은 Timer/Counter를 이용하여 OC1A출력 주파수를 조절해 원하는 음을 재생시켜 음악을 연주하는 프로그램이다
//연주하는 곡은 '고향의 봄'이며, 연주 방식은 play_music 함수와 rest함수를 통해 각 음표의 음높이와 음길이를 결정해 음악을 재생한다
//음악은 Peripheral Board의 Buzzer를 통해 재생된다
// 
//동작은 다음과 같다
//1. Timer/Counter를 CTC 모드로 설정한다
//2. OC1A가 Compare match되면 toggle이 되도록 설정한다
//3. 주파수 설정을 위해 prescaler는 8로 설정한다
//4. play_music함수를 통해 원하는 음을 원하는 길이로 재생힌다
//5. 쉼표가 필요한 경우는 쉽표를 구현하는 함수인 rest로 쉼표를 구현한다

//<Arduino MEGA 2560 - Peripheral Board 결선>
//Arduino MEGA 2560		Peripheral Board
//	PB5(11)					LED8(Buzzer)

void play_music(uint16_t scale, int length); // 원하는 음을 재생하는 함수 구현, parameter는 음의 주파수, 재생 길이
void rest(int length); // 쉼표를 구현하는 함수, parameter는 재생길이
uint16_t scale_C = 3821; // C(도)의 주파수를 만들기 위한 OCR1A값
uint16_t scale_D = 3404; // D(레)의 주파수를 만들기 위한 OCR1A값
uint16_t scale_E = 3033; // E(미)의 주파수를 만들기 위한 OCR1A값
uint16_t scale_F = 2863; // F(파)의 주파수를 만들기 위한 OCR1A값
uint16_t scale_G = 2550; // G(솔)의 주파수를 만들기 위한 OCR1A값
uint16_t scale_A = 2272; // A(라)의 주파수를 만들기 위한 OCR1A값
uint16_t scale_B = 2024; // B(시)의 주파수를 만들기 위한 OCR1A값
uint16_t scale_highC = 1910; // 높은 C(높은 도)의 주파수를 만들기 위한 OCR1A값
uint16_t scale_highD = 1702; // 높은 D(높은 레)의 주파수를 만들기 위한 OCR1A값
uint16_t scale_highE = 1520; // 높은 E(높은 레)의 주파수를 만들기 위한 OCR1A값

int whole = 2727; // 온음표의 재생시간인 2.727초를 ms 단위로 표현
int half = 1364; // 2분음표의 재생시간인 1.364초를 ms 단위로 표현
int dot_half = 2045; // 점 2분음표의 재생시간인 2.045초를 ms 단위로 표현
int quarter = 682; // 4분음표의 재생시간인 0.682초를 ms 단위로 표현
int note_8th = 341; // 8분음표의 재생시간인 0.341초를 ms 단위로 표현
int quarter_rest = 682; // 4분쉼표의 재생시간인 0.682초를 ms 단위로 표현

void setup(){
	DDRB |= 0b00100000; // OC1A가 연결된 PB5를 Output으로 설정
	TCCR1A &= ~_BV(WGM10); //CTC 모드 설정
	TCCR1A &= ~_BV(WGM11);
	TCCR1B &= ~_BV(WGM13);
	TCCR1B |= _BV(WGM12);
	
	TCCR1A &= ~_BV(COM1A1); // ouput comapare match 발생 시 OC1A가 toggle되도록 설정
	TCCR1A |= _BV(COM1A0);

	
	
	TCCR1B &= ~_BV(CS10); // prescaler를 8로 설정
	TCCR1B &= ~_BV(CS12);
	TCCR1B |= _BV(CS11);

	TCNT1 = 0; // counter의 초기값을 0으로 초기화
}

void loop(){
	//첫 번째 줄 재생
	play_music(scale_G, quarter);
	play_music(scale_G, quarter);
	play_music(scale_E, note_8th);
	play_music(scale_F, note_8th);
	play_music(scale_G, quarter);

	play_music(scale_A, quarter);
	play_music(scale_A, quarter);
	play_music(scale_G, half);

	play_music(scale_G, quarter);
	play_music(scale_highC, quarter);
	play_music(scale_highE, quarter);
	play_music(scale_highD, note_8th);
	play_music(scale_highC, note_8th);
	
	play_music(scale_highD, dot_half);
	rest(quarter_rest);
	
	//두 번째 줄 재생
	play_music(scale_highE, quarter);
	play_music(scale_highE, quarter);
	play_music(scale_highD, quarter);
	play_music(scale_highD, quarter);
	
	play_music(scale_highC, quarter);
	play_music(scale_highD, note_8th);
	play_music(scale_highC, note_8th);
	play_music(scale_A, quarter);
	play_music(scale_A, quarter);

	play_music(scale_G, quarter);
	play_music(scale_G, quarter);
	play_music(scale_G, quarter);
	play_music(scale_E, note_8th);
	play_music(scale_D, note_8th);

	play_music(scale_C, dot_half);
	rest(quarter_rest);

	//세 번째 줄 재생
	play_music(scale_D, quarter);
	play_music(scale_D, quarter);
	play_music(scale_E, quarter);
	play_music(scale_C, quarter);

	play_music(scale_D, quarter);
	play_music(scale_D, quarter);
	play_music(scale_E, quarter);
	play_music(scale_G, quarter);

	play_music(scale_A, quarter);
	play_music(scale_highC, quarter);
	play_music(scale_highE, quarter);
	play_music(scale_highD, note_8th);
	play_music(scale_highC, note_8th);

	play_music(scale_highD, dot_half);
	rest(quarter_rest);

	//네 번째 줄 재생
	play_music(scale_highE, quarter);
	play_music(scale_highE, quarter);
	play_music(scale_highD, quarter);
	play_music(scale_highD, quarter);

	play_music(scale_highC, quarter);
	play_music(scale_highD, note_8th);
	play_music(scale_highC, note_8th);
	play_music(scale_A, quarter);
	play_music(scale_A, quarter);

	play_music(scale_G, quarter);
	play_music(scale_G, quarter);
	play_music(scale_G, quarter);
	play_music(scale_E, note_8th);
	play_music(scale_D, note_8th);

	play_music(scale_C, dot_half);
	rest(quarter_rest);
}

void play_music(uint16_t scale, int length) { // 음표 재생을 하는 함수
	TCCR1A &= 0b00111111; // 재생하기 위해 상위 2비트만 0으로 만들기
	TCCR1A |= 0b01000000; // COM1A0을 1로 설정해서 toggle이 가능하도록 한다

	OCR1A = scale; // 주파수 설정을 위한 OCR1A(Top value)설정
	delay(length); // 해당 음을 음표 길이만큼 재생
	TCCR1A &= ~0b11000000; // OC1A를 output pin으로 연결하지 않으면 음악이 재생되지 않는다
	TCCR1A |= 0b00000000; // 따라서 상위 2비트를 0으로 만들어줘서 음 사이의 간격을 만들어준다
	delay(length/40); // 음 사이의 간격을 만들어준다
}

void rest(int length) { // 쉼표 함수
	TCCR1A &= ~0b11000000; // OC1A를 PB5에 연결하지 않도록 상위 2비트 초기화
	TCCR1A |= 0b00000000; // 상위 2비트를 0으로 만들어서 OC1A가 PB5에 연결되지 않도록 해서 음악이 재생되지 않도록 한다
	delay(length); // 쉼표의 시간만큼 기다린다
}