#include <avr/io.h>
#define F_CPU 16000000UL	//CPU CLOCK

int main()
{
	char x = 0;
	char y = 0;
	char sub = 0;	//- 스위치가 눌렸는지 여부
	char sw[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };	//switch 배열
	DDRA = 0xff;	//PORTA를 출력 모드 (LED)
	PORTA = 0x00;	//모든 LED가 꺼진 상태
	DDRB = 0x00;	//PORTB를 입력 모드 (SWTICH)
	DDRG = ~0x0c; // 2번 비트와 3번 비트 입력모드
	while (1)
	{
		if (!sub)	//x를 입력 받는 부분
		{
			for (int i = 0; i < 8; i++)	//switch의 입력을 확인
			{
				if (PINB & sw[i])
				{
					while (PINB & sw[i]);
					if (x & sw[i])	//LED가 켜져있을 때
					{
						x &= ~(sw[i]);	//clear
						PORTA = x;
					}
					else			//LED가 꺼져있을 때
					{
						x |= sw[i];	//set
						PORTA = x;
					}
				}
			}
		}
		else	//- 스위치를 누른 뒤 y를 입력 받는 부분
		{
			for (int i = 0; i < 8; i++)	//switch의 입력을 확인
			{
				if (PINB & sw[i])
				{
					while (PINB & sw[i]);
					if (y & sw[i])	//LED가 켜져있을 때
					{
						y &= ~(sw[i]);	//clear
						PORTA = y;
					}
					else			//LED가 꺼져있을 때
					{
						y |= sw[i];	//set
						PORTA = y;
					}
				}
			}
		}
		if (PING & 0x04)	//- 스위치 (PORTG 2번 비트)
		{
			while (PING & 0x04);
			sub = 1;	//- 스위치 입력 여부 On
			PORTA = y;	//LED는 y로 전환
		}
		if (PING & 0x08)	//= 스위치 (PORTG 3번 비트)
		{
			while (PING & 0x08);
			PORTA = x - y;	//LED에 x - y 결과 출력
		}
	}
}
