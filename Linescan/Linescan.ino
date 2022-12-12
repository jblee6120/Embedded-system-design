#define CLK PA0
#define SI PA1
# define PI 3.141592654

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

int flag = 0;
char a;
int s_cnt = 0;

float ts = 0.01;
float time = 0.0;
uint32_t MicrosSampleTime;
uint32_t time_out;

void ADCInit();

void setup() {
	//
	LPF1 = new LPF(3.0, ts);
	//

	Serial.begin(115200);
	ADCInit();

	pinMode(13, OUTPUT);
	DDRA = (1 << CLK) | (1 << SI);
	
	MicrosSampleTime = (uint32_t)(ts * 1e6);
	time_out = micros() + MicrosSampleTime;
}

void loop() {
	int i;
	time += ts;
	digitalWrite(13, HIGH);

	if (Serial.available()) {
		a = Serial.read();
		if (a == 'A') flag = 1;

	}

	PORTA |= (1 << SI);
	PORTA |= (1 << CLK);
	PORTA &= ~(1 << SI);

	delay(1);

	for (i = 0; i < 129; i++) {
		ADC_Get(i);
		PORTA &= ~(1 << CLK);
		PORTA |= (1 << CLK);
	}
	PORTA &= ~(1 << CLK);

	if (flag == 1) {
		for (i = s_cnt * 20; i < (s_cnt + 1) * 20; i++) {
			Serial.print(filter_data[i]);
			Serial.print(",");

			if (i > 127) break;
		}
		Serial.println(" ");

		s_cnt++;
		if (s_cnt >= 7) {
			s_cnt = 0;

			Serial.print('S');
		}
	}
	digitalWrite(13, LOW);

	while (!((time_out - micros()) & 0x80000000));
	time_out += MicrosSampleTime;
}

void ADCInit() {
	ADMUX = 0;
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

void ADC_Get(unsigned char num) {
	ADCSRA |= (1 << ADSC);

	while (ADCSRA & (1 << ADSC));
	camera_data[num] = ADCW;
	LPF1->uk = camera_data[num];
	LPF1->calc();
	filter_data[num] = (int)LPF1->yk;
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