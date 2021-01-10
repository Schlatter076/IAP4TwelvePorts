/*
 * L74HC595.c
 *
 *  Created on: 2021年1月5日
 *      Author: HHS007
 */
#include "L74HC595.h"

struct HC595Fram HC595_STATUS =
{ 0 };

void L74HC595_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	_HC595_CLK = 0;
	_HC595_LATCH = 0;
	_HC595_DATA = 0;
	memset(HC595_STATUS.LAST_LED_STATU, 0xFF, 3); //初始全灭
	HC595_Send_Multi_Bytes(HC595_STATUS.LAST_LED_STATU, 3);
}

/***
 *74HC595 发送一个字节
 *即往74HC595的SI引脚发送一个字节
 */
void HC595_Send_Byte(u8 byte)
{
	u8 i;
	for (i = 0; i < 8; i++)  //一个字节8位，传输8次，一次一位，循环8次，刚好移完8位
	{
		/****  步骤1：将数据传到DS引脚    ****/
		if (byte & 0x80)        //先传输高位，通过与运算判断第八是否为1
			_HC595_DATA = 1;    //如果第八位是1，则与 595 DS连接的引脚输出高电平
		else
			//否则输出低电平
			_HC595_DATA = 0;

		/*** 步骤2：SHCP每产生一个上升沿，当前的bit就被送入移位寄存器 ***/
		_HC595_CLK = 0;   // SHCP拉低
		delay_us(1);           // 适当延时
		_HC595_CLK = 1;  // SHCP拉高， SHCP产生上升沿
		delay_us(1);
		byte <<= 1;		// 左移一位，将低位往高位移，通过	if (byte & 0x80)判断低位是否为1
	}
	//输出锁存使能
//	_HC595_LATCH = 0;
//	delay_us(1);
//	_HC595_LATCH = 1;
//	delay_us(1);
}

void HC595_Send_Multi_Bytes(u8 *bytes, u16 len)
{
	for (int i = 0; i < len; i++)
	{
		HC595_Send_Byte(bytes[i]);
	}
	//输出锁存使能
	_HC595_LATCH = 0;
	delay_us(1);
	_HC595_LATCH = 1;
	delay_us(1);
}

void ledON(u8 led)
{
	if (led < 5)
	{
		HC595_STATUS.LAST_LED_STATU[0] &= ~(1 << (led - 1));
	}
	else if (led < 9)
	{
		HC595_STATUS.LAST_LED_STATU[1] &= ~(1 << (led - 1));
	}
	else
	{
		HC595_STATUS.LAST_LED_STATU[2] &= ~(1 << (led - 1));
	}
	HC595_Send_Multi_Bytes(HC595_STATUS.LAST_LED_STATU, 3);
}
void ledOFF(u8 led)
{
	if (led < 5)
	{
		HC595_STATUS.LAST_LED_STATU[0] |= (1 << (led - 1));
	}
	else if (led < 9)
	{
		HC595_STATUS.LAST_LED_STATU[1] |= (1 << (led - 1));
	}
	else
	{
		HC595_STATUS.LAST_LED_STATU[2] |= (1 << (led - 1));
	}
	HC595_Send_Multi_Bytes(HC595_STATUS.LAST_LED_STATU, 3);
}
void ledBLINK(u8 led)
{
	if (led < 5)
	{
		HC595_STATUS.LAST_LED_STATU[0] ^= (1 << (led - 1));
	}
	else if (led < 9)
	{
		HC595_STATUS.LAST_LED_STATU[1] ^= (1 << (led - 1));
	}
	else
	{
		HC595_STATUS.LAST_LED_STATU[2] ^= (1 << (led - 1));
	}
	HC595_Send_Multi_Bytes(HC595_STATUS.LAST_LED_STATU, 3);
}
