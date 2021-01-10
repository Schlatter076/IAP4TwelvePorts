/*
 * L74HC595.h
 *
 *  Created on: 2021年1月5日
 *      Author: HHS007
 */

#ifndef L74HC595_L74HC595_H_
#define L74HC595_L74HC595_H_

#include "sys.h"
#include "delay.h"
#include <string.h>

#define _HC595_CLK    (PAout(6))
#define _HC595_LATCH  (PAout(5))
#define _HC595_DATA   (PAout(4))

extern struct HC595Fram
{
	u8 LAST_LED_STATU[3];
	u8 slowBLINK[12];
	u8 slowCnt[12];
	u8 fastBLINK[12];
	u8 fastCnt[12];
} HC595_STATUS;

void L74HC595_Init(void);
void HC595_Send_Byte(u8 byte);
void HC595_Send_Multi_Bytes(u8 *bytes, u16 len);
void ledON(u8 led);
void ledOFF(u8 led);
void ledBLINK(u8 led);

#endif /* L74HC595_L74HC595_H_ */
