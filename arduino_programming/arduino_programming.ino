//12181407 이종범 임베디드시스템설계 과제1
//
//File name : arduino_programming.ino
//<설명>
//이 프로그램은 Arduino Library를 이용해 pin의 output을 제어하는 프로그램이다.
//작동방식은 LED의 갯수에 따라 4 type, LED가 움직이는 속도에 따라 4단계의 속도가 있다.
//
//a type : 1개의 LED가 움직이는 타입이다.
//b type : 2개의 LED가 움직이는 타입이다.
//c type : 3개의 LED가 움직이는 타입이다.
//d type : 4개의 LED가 움직이는 타입이다.
//
//1단계 : 500ms마다 LED가 한 칸씩 움직이도록 한다.
//2단계 : 250ms마다 LED가 한 칸씩 움직이도록 한다.
//3단계 : 125ms마다 LED가 한 칸씩 움직이도록 한다.
//4단계 : 62ms마다 LED가 한 칸씩 움직이도록 한다.
//
//<Arduino MEGA 2560 - Peripheral Board간의 연결>
//		Arduino MEGA 2560			Peripheral Board
//
//			Pin 14						LED 1
//			Pin 15						LED 2
//			Pin 16						LED 3
//			Pin 17						LED 4
//			Pin 18						LED 5
//			Pin 19						LED 6
//			Pin 20						LED 7
//			Pin 21						LED 8
//
//<입출력설정>
//Arduino MEGA 2560의 14~21번 Pin을 OUTPUT으로 설정해 LED에 출력을 입력할 수 있도록 한다.

char data1[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 }; //실제 Arduino가 출력해야하는 Pin의 상태를 담은 배열
char data_a[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 }; //a type의 출력순서를 담은 배열
char data_b[8] = { 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x81 }; //b type의 출력순서를 담은 배열
char data_c[8] = { 0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0, 0xC1, 0x83 }; //c type의 출력순서를 담은 배열
char data_d[8] = { 0x0F, 0x1E, 0x3C, 0x78, 0xF0, 0xE1, 0xC3, 0x87 }; //d type의 출력순서를 담은 배열
int count = 0; //출력배열의 인덱스를 담는 변수
char data_in; //LED 제어를 하기위한 입력값을 담는 변수

int t = 500; //LED의 이동시간을 정하는 변수
void setup()
{
	Serial.begin(115200); //Serial통신을 baudrate 115200bps로 초기화
	for (int i = 14; i < 22; i++) pinMode(i, OUTPUT); //14~21번 Pin을 OUTPUT으로 설정
}

void loop()
{
	if (Serial.available() > 0) data_in = Serial.read(); //버퍼에 시리얼통신을 통한 입력이 들어오면 data_in에 입력값을 담는다

	if (data_in == 'a') { //입력값이 a인 경우
		for (int i = 0; i < 8; i++) data1[i] = data_a[i]; //data1 (실제 출력)배열을 data_a (a type 출력)배열로 갱신
	}

	else if (data_in == 'b') { //입력값이 b인 경우
		for (int i = 0; i < 8; i++) data1[i] = data_b[i]; //data1 (실제 출력)배열을 data_b (b type 출력)배열로 갱신
	}

	else if (data_in == 'c') { //입력값이 b인 경우
		for (int i = 0; i < 8; i++) data1[i] = data_c[i]; //data1 (실제 출력)배열을 data_c (c type 출력)배열로 갱신
	}

	else if (data_in == 'd') { //입력값이 b인 경우
		for (int i = 0; i < 8; i++) data1[i] = data_d[i]; //data1 (실제 출력)배열을 data_d (d type 출력)배열로 갱신
	}

	else if (data_in == '1') { //입력값이 1인 경우
		t = 500; //LED가 한 칸 이동하는 시간을 500ms로 변경
	}

	else if (data_in == '2') { //입력값이 2인 경우
		t = 250; //LED가 한 칸 이동하는 시간을 250ms로 변경
	}

	else if (data_in == '3') { //입력값이 3인 경우
		t = 125; //LED가 한 칸 이동하는 시간을 125ms로 변경
	}

	else if (data_in == '4') { //입력값이 4인 경우
		t = 62; //LED가 한 칸 이동하는 시간을 62ms로 변경
	}


	for (int i = 14; i < 22; i++) digitalWrite(i, data1[count] & 1 << (i - 14));
	//for문 설명 : for문으로 14~21번 Pin을 돌면서 Bitwise and를 통해 data1[count]의 각 비트상태에 따라 14~21번 Pin의 출력을 결정한다.
	//i - 14를 쓴 이유: Pin 번호는 14~21이지만, LED 상태를 담는 비트는 8비트이므로 비트 시프트의 범위를 0~7로 변경하기 위함.
	count++; //for문이 끝나면 1개 상태의 LED출력이 끝났으므로 다음 상태로 넘어가기 위해 인덱스 count를 1 증가시킨다.
	if (count == 8) count = 0; //data1의 인덱스가 0~7이므로 count가 8이되면 0으로 초기화시킨다.
	delay(t); //t ms만큼 대기 -> LED가 움직이는 속도를 결정
}