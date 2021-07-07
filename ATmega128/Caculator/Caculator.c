#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL	//CPU CLOCK
#include <util/delay.h>		//<avr/delay.h> has been moved to <util/delay.h>.

#define DELAY 1	//segment display delay (ms)
char number[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x67 };	//7-Segment number
volatile float count = 0;
int min = 0;		//딜레이를 위해 카운트할 min 초기화
int i = 0;			//자릿수를 뒤로 넘기는 변수

ISR(INT4_vect)	//INT4 interrupt service routine, 천의 자리
{
	if ((int)count / 1000 == 9)	//9를 넘기면 0으로 초기화
		count -= 9000;
	else
		count += 1000;
}
ISR(INT5_vect)	//INT5 interrupt service routine, 백의 자리
{
	if (((int)count % 1000) / 100 == 9)
		count -= 900;
	else
		count += 100;
}
ISR(INT6_vect)	//INT6 interrupt service routine, 십의 자리
{
	if (((int)count % 100) / 10 == 9)
		count -= 90;
	else
		count += 10;
}
ISR(INT7_vect)	//INT7 interrupt service routine, 일의 자리
{
	if ((int)count % 10 == 9)
		count -= 9;
	else
		count++;
}

void PrintLED(float value)	//일반적인 4-digit 7-segment를 출력하는 함수, 0~9999 정수
{
	long tmp;
	int digit;
	long a = 1;		//한 자리씩 끊어 읽을 때 활용하는 변수
	if (value < 0)	//음수일 때
	{
		PORTC = ~0x01;
		PORTA = 0x40;	//digit 3, - 표시
		if (i < 4)		//4자리 이내
		{
			a = 10;
			for (int j = 0; j < 3 - i; j++)
				a *= 10;	//각 자리 숫자를 구하기 위해 사용하는 변수
			value *= -1;	//자연수의 value 값만
			if (min < 100)	//delay
			{
				for (int k = 0; k < 3; k++, a /= 10)
				{
					tmp = ((long)value % (a)) / (a / 10);//가장 앞 자리부터 시작
					PORTC = ~(0x02 << k);		//digit 2 - k
					if (tmp >= 0 && tmp <= 9)	//숫자가 없으면 공백 표시
						PORTA = number[tmp];
					else
						PORTA = 0x00;
					_delay_ms(DELAY);
				}
				min++;
			}
			else
			{
				min = 0;	//딜레이 초기화
				i++;	//자릿수 넘어가기		
			}
		}
		else
		{
			min = 0;		//초기화
			i = 0;		//반복이 끝나고 다시 보여주기 위해 초기화
		}
	}
	else if (value >= 10000)
	{
		digit = 0;			//자릿수 초기화
		tmp = (long)value;	//옮겨 담는다.
		while (tmp > 0)		//tmp를 10으로 나눠보면서 자릿수 구하기
		{
			tmp /= 10;
			digit++;		//자릿수를 구한다.
		}
		if (i <= digit)		//자릿수만큼 반복
		{
			a = 1;
			for (int j = 0; j < digit - i; j++)
				a *= 10;	//각 자리를 구하기 위해 사용하는 변수

			if (min < 100)	//delay
			{
				for (int k = 0; k < 4; k++, a /= 10)
				{
					tmp = ((long)value % (a)) / (a / 10);//가장 앞 자리부터 시작
					PORTC = ~(0x01 << k);		//digit 3 - k
					if (tmp >= 0 && tmp <= 9)	//숫자가 없으면 공백 표시
						PORTA = number[tmp];
					else
						PORTA = 0x00;
					_delay_ms(DELAY);
				}
				min++;
			}
			else
			{
				min = 0;	//딜레이 초기화
				i++;	//자릿수 넘어가기		
			}
		}
		else
		{
			min = 0;		//초기화
			i = 0;		//반복이 끝나고 다시 보여주기 위해 초기화
		}
	}
	else
	{
		a = 10000;
		for (int k = 0; k < 4; k++)
		{
			tmp = ((long)value % (a)) / (a / 10);//가장 앞 자리부터 시작
			PORTC = ~(0x01 << k);
			if (tmp >= 0 && tmp <= 9)	//숫자가 없으면 공백 표시
				PORTA = number[tmp];	//digit 3-k
			else
				PORTA = 0x00;
			_delay_ms(DELAY);
			a /= 10;
		}
	}
}

int main()
{
	int x, y;		//4자리수인 x, y 변수
	long tmp;		//7-segment에 출력할 때 판단하는 각 자리의 숫자
	int touch = 0;	//sw1을 누른 횟수
	float value;	//count를 옮겨담을 변수
	count = 0;		//count 초기화
	x = 0;			//x 초기화
	y = 0;			//y 초기화
	long a = 1;		//한 자리씩 끊어 읽을 때 활용하는 변수

	DDRC = 0x0f;						//Segment digit
	DDRA = 0xff;						//Segment number
	DDRG = ~0x0c; 					// 2번 비트와 3번 비트 입력모드
	EICRB = (EICRB & 0x00) | 0xaa;	//INT4:7 falling edge trigger
	EIMSK |= 0xf0;					//enable INT4:7
	EIFR = 0xff;					//clear interrupt flag
	sei();							//enable global interrupt

	while (1)
	{
		if (PING & 0x04)	//PORTG 2번 비트
		{
			while (PING & 0x04);
			x = count;		//count를 x에 저장
			count = 0;		//count 초기화
			y = 0;			//y 초기화
			touch = 0;		//touch 횟수 초기화
		}
		if (PING & 0x08)	//PORTG 3번 비트
		{
			while (PING & 0x08);
			switch (touch)
			{
			case 0:			//0번째 touch
				y = count;	//count를 y에 저장
				count = x + y;	//덧셈하여 출력
				touch++;	//touch 횟수 증가
				break;
			case 1:
				count = x - y;	//뺄셈하여 출력
				touch++;	//touch 증가
				break;
			case 2:
				count = (long)x * (long)y;	//type casting하여 곱셈
				touch++;
				break;
			case 3:
				count = (float)x / (float)y;	//type casting하여 나눗셈
				touch++;
				break;
			default:
				count = 0;	//초기화
				x = 0;
				y = 0;
				touch = 0;
				break;
			}
			i = 0;			//자릿수 초기화
			min = 0;		//딜레이 초기화
		}

		if (touch == 4 && y == 0)	//0으로 나눌 때 Err 출력
		{
			PORTC = ~0x01;
			PORTA = 0x00;	//digit 3, 공백
			_delay_ms(DELAY);

			PORTC = ~0x02;
			PORTA = 0x79;	//digit 2, E
			_delay_ms(DELAY);

			PORTC = ~0x04;
			PORTA = 0x50;	//digit 1, r
			_delay_ms(DELAY);

			PORTC = ~0x08;
			PORTA = 0x50;	//digit 0, r
			_delay_ms(DELAY);
		}
		else if (touch == 4)	//나눴을 때
		{
			if (((long)(count * 100)) % 10 >= 5)	//소수점 아래 둘째 자리에서 반올림
				value = (long)(count * 10) + 1;
			else
				value = (long)(count * 10);

			a = 10000;
			for (int k = 0; k < 4; k++, a /= 10)
			{
				tmp = ((long)value % a) / (a / 10);
				PORTC = ~(0x01 << k);
				PORTA = number[tmp];	//digit k-1
				if (k == 2)
					PORTA |= 0x80;		//소수점 표시
				_delay_ms(DELAY);
			}
		}
		else
			PrintLED(count);	//일반적인 정수를 출력하는 함수
	}
}  
