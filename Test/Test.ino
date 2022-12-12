# define PI 3.141592654
unsigned short data;
float sample_time = 0.02;
uint32_t start_time;
uint32_t MicrosSampleTime;

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

void setup()
{
	Serial.begin(115200);

	LPF1 = new LPF(3.0, sample_time);

	MicrosSampleTime = (uint32_t)(sample_time * 1e6);
	start_time = micros() + MicrosSampleTime;
}
void loop()
{
	data = analogRead(A0);

	LPF1->uk = (float)data;
	LPF1->calc();

	Serial.print(data);
	Serial.print(" ");
	Serial.println(LPF1->yk);

	while (!((start_time - micros()) & 0x80000000));
	start_time += MicrosSampleTime;
}

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