#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL	//CPU CLOCK, 16MHz

char LEDdigit[4] = { 0, 0, 0, 0 };
volatile int count = 0;		//count 초기화
volatile long time = 0;		//time 초기화 (s)
volatile int year = 2021;	//시작년도 2021년
int touch = 0;
void PrintTime(long num);
void PrintToday(long num);
void PrintSec(long num);
void PrintYear(int year);
void PrintWeek(long num);

//256 * 8 microseconds = 2.048 ms
ISR(TIMER0_OVF_vect)	//timer0 overflow interrupt
{
	static char number[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x67 };	//7-Segment number
	static char index = 0;
	index++;
	index &= 0x3;
	PORTC = ~(1 << (3 - index));
	PORTA = number[LEDdigit[index]];
	if (index == 2)
		PORTA |= 0x80;	//점으로 2자리씩 분할 표시
}

ISR(TIMER2_COMP_vect)	//timer2 output compare match interrupt
{
	count++;
	if (count == 500)	//2ms*500 = 1s
	{
		time++;
		if ((!(year % 4) && (year % 100)) || !(year % 400))	//윤년인지 판단하고 해 넘기기
		{
			if (time >= 31622400)	//3600*24*366일
			{
				year++;
				time -= 31622400;
			}
		}
		else
		{
			if (time >= 31536000)	//3600*24*365일
			{
				year++;
				time -= 31536000;
			}
		}
		count = 0;
		if (touch == 1)
			PrintToday(time);
		else if (touch == 2)
			PrintSec(time);
		else if (touch == 3)
			PrintYear(year);
		else if (touch == 4)
			PrintWeek(time);
		else
			PrintTime(time);
	}
}



int main()
{
	DDRC = 0x0f;		//Segment digit
	DDRA = 0xff;		//Segment number
	DDRG = ~0x0c;		//2번 비트와 3번 비트 입력모드
	TCNT0 = 0x00;		//clear
	TCNT2 = 0x00;		//clear
	TCCR0 = 0b00000101;	//Normal mode, disable OC0, 1/128 (8 microseconds)
	TCCR2 = 0b00001100;	//CTC mode, 1/256 (16 microseconds)
	OCR2 = 124;			//16 (microseconds) * 125 = 2ms
	TIMSK = 0b10000001;	//timer0 overflow interrupt, timer2 output compare match interrupt
	sei();				//enable global interrupt

	while (1)
	{
		if (PING & 0x04)	//PORTG 2번 비트
		{
			while (PING & 0x04);
			switch (touch)
			{
			case 0:
				PrintToday(time);	//0번째 touch, 월, 일 출력
				touch++;
				break;
			case 1:
				PrintSec(time);	//1번째, 초 출력
				touch++;
				break;
			case 2:
				PrintYear(year);	//2번째, 년 출력
				touch++;
				break;
			case 3:
				PrintWeek(time);	//3번째, 요일 출력
				touch++;
				break;
			default:
				PrintTime(time);	//원래대로 시, 분 출력
				touch = 0;
				break;
			}
		}
		if (PING & 0x08)	//PORTG 3번 비트, 초기화
		{
			while (PING & 0x08);
			count = 0;
			time = 0;
			touch = 0;
			year = 2021;
			PrintTime(time);
		}
	}
}


void PrintTime(long num)	//현재 시각의 (시, 분)
{
	num %= 86400;		//하루는 86400초
	int hour = num / 3600;		//1시간은 3600초
	int min = (num % 3600) / 60;	//1분은 60초
	LEDdigit[3] = hour / 10;
	LEDdigit[2] = hour % 10;
	LEDdigit[1] = min / 10;
	LEDdigit[0] = min % 10;
}
void PrintToday(long num)	//오늘 날짜 (월, 일)
{
	num = num / 86400;		//하루는 86400초, 1월 1일부터 시작
	int month = 0;
	char monthday[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };	//2021년 기준
	if ((!(year % 4) && (year % 100)) || !(year % 400))
		monthday[1] = 29;		//윤년이면 2월 29일까지
	else
		monthday[1] = 28;
	while (num >= monthday[month])
	{
		num -= monthday[month++];
	}
	month++;			//사람은 1월부터 시작
	num++;			//1일부터 시작
	LEDdigit[3] = month / 10;
	LEDdigit[2] = month % 10;
	LEDdigit[1] = num / 10;
	LEDdigit[0] = num % 10;
}
void PrintSec(long num)		//현재 시각 (초)
{
	int sec = num % 60;
	LEDdigit[3] = 0;
	LEDdigit[2] = 0;
	LEDdigit[1] = sec / 10;
	LEDdigit[0] = sec % 10;
}
void PrintYear(int year)		//올해 출력
{
	LEDdigit[3] = year / 1000;
	LEDdigit[2] = (year % 1000) / 100;
	LEDdigit[1] = (year % 100) / 10;
	LEDdigit[0] = year % 10;
}
void PrintWeek(long num)
{
	num = num / 86400 + 5;		//하루는 86400초, 1월 1일부터 시작, 기준인 2021년 1월 1일은 금요일
	for (int i = 2021; year - i > 0; i++)			//2021년 1월 1일부터 지난 날짜를 더한다.
	{
		if ((!(i % 4) && (i % 100)) || !(i % 400))	//윤년이면 366일
			num += 366;
		else
			num += 365;
	}
	num %= 7;			//지난 날짜에 7을 나눈 나머지는 요일이다.
	LEDdigit[3] = 0;
	LEDdigit[2] = 0;
	LEDdigit[1] = 0;
	LEDdigit[0] = num;	//week = {일, 월, 화, 수, 목, 금, 토}
}

