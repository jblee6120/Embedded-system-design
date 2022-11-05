// Youtube Link: https://youtu.be/8VPjYkcU5eU
// 12181407 이종범 임베디드시스템설계 class 과제
// 
//File name: 12181407_delay.ino
// 
//<설명>
//이 프로그램은 c++의 class를 사용하여 potentiometer에서 나오는 
//전압출력을 0.3초 delay시키는 기능을 수행한다
//
//<Arduiono MEGA 2560 - Peripheral Board 결선>
//Arduino MEGA 2560		Peripheral Board
//		A0					ANA1(Potentiometer)
//
//동작은 다음과 같다
//1. delay 구현을 위한 클래스를 정의한다
//2. 주기적 연산 수행을 위한 sampling time, 현재 시간을 갖는 변수를 생성한다
//3. analogRead()를 통해 ANA1에서 ADC값을 받아온다
//4. class의 member function중 0.3초 delay된 결과를 출력하는 함수를 호출한다.
//5. 호출한 함수의 출력과 raw data를 시리얼 통신으로 출력해준다
//6. 출력 결과를 Serial plot으로 확인한다


float sample_time = 0.01; // 0.01초마다 주기적 연산을 수행하기 위한 sampling time(sec)변수를 설정
uint32_t start_time; // 주기적 연산을 시작하는 시간을 담는 변수선언
uint32_t MicrosSampleTime; // 0.01sec의 sample_time을 us단위로 변환한 값을 담는 변수
int data; // ADC값을 받는 변수


class Delay // Delay기능을 수행하는 Delay class 선언
{
public: // 접근이 가능하도록 한다
	Delay(); // 클래스의 생성자 선언. 생성자 함수는 클래스 명과 동일한 이름을 갖는다
	~Delay(); // 클래스의 소멸자 선언. 소멸자 함수는 ~클래스명이다
	float delayed_result(); //0.3초 delay된 결과를 출력하는 멤버 함수

public:
	float buffer[31] = { 0, }; // delay된 값을 출력하기 위해서는 과거의 값을 출력해야 함. 따라서 과거의 값을 저장할 버퍼 배열을 정의
	float input_data; // ADC값을 받아서 멤버함수에서 처리되는 변수
};

Delay *Delay_Test; // 포인터로 Delay_Test라는 이름의 class 선언

void setup(){
	Serial.begin(115200); // baudrate 115200으로 시리얼 통신을 준비한다
	Delay_Test = new Delay(); // Delay라는 class의 형태를 갖는 객체 Delay_Test를 만든다

	MicrosSampleTime = (uint32_t)(sample_time * 1e6); // 0.01sec의 sample_time을 us단위로 변환해서 저장한다 
	start_time = micros() + MicrosSampleTime; // 다음 주기적 연산의 시작시간은 us단위의 현재시간 + 샘플링시간
}

void loop(){
	data = analogRead(A0); // ANA1에서 ADC 값을 받아온다
	Delay_Test->input_data = (float)data; // ADC 값을 float으로 형변환해서 Delay_Test의 멤버변수 input_data에 입력한다
	

	Serial.print((float)data); // raw data를 출력한다
	Serial.print(" "); // 데이터 구분을 위한 스페이스 출력
	Serial.println(Delay_Test->delayed_result()); // 0.3초 딜레이가 된 결과를 리턴하는 함수를 호출해서 0.3초 딜레이된 결과 출력

	while (!((start_time - micros()) & 0x80000000)); // 현재시간이 새로운 주기적연산을 시작하는 시간을 넘어갈때까지 대기 
	start_time += MicrosSampleTime; // 새로운 주기적 연산을 시작하는 시간을 갱신한다

}

Delay::Delay() { // 생성자 함수의 정의
	
}

float Delay::delayed_result() { // Delay 클래스에 속하는 delay_result()함수의 정의, 기능은 0.3초 딜레이된 결과를 출력한다

	buffer[30] = input_data; // 가장 마지막 버퍼에 ADC값을 받는다

	for (int i = 0; i <= 30; i++) { 
		buffer[i-1] = buffer[i]; // 0.01초 단위로 기록된 값들을 한 칸씩 밀면서 과거의 값을 저장한다
								 // 배열 성분 하나당 0.01초 이전의 값을 저장하기 때문에 가장 마지막인 buffer[0] 0.01*30 = 0.3초 이전의 값을 갖는다
	}

	return buffer[0]; // 0.3초 이전의 값을 갖는 buffer[0]을 리턴한다
}

Delay::~Delay() // 소멸자
{

}