/*
 * tcp_public.h
 *
 *  Created on: 2021年1月5日
 *      Author: HHS007
 */

#ifndef _TCP_PUBLIC_H_
#define _TCP_PUBLIC_H_

#include "sys.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include "delay.h"
#include "malloc.h"
#include "STMFlash.h"
#include "delay.h"
#include "base64.h"
#include "F4G.h"
#include "wifi.h"

#define TCP_MAX_LEN 1024		  //最大接收缓存字节数
#define BASE64_BUF_LEN 512

struct STRUCT_USART_Fram  //定义一个全局串口数据帧的处理结构体
{
	unsigned char RxBuf[TCP_MAX_LEN];
	unsigned char TxBuf[BASE64_BUF_LEN];
	__IO u8 Online;
	__IO u8 firstStatuHeartNotSucc;
	__IO u8 allowHeart;
	__IO u8 AT_test_OK;
	vu8 DMA_Tx_Busy;
	vu16 AccessLen;
	vu8 FinishFlag;
};
extern struct STRUCT_USART_Fram F4G_Fram;
extern struct STRUCT_USART_Fram WIFI_Fram;
extern struct STRUCT_USART_Fram USART1_Fram;
//===================枚举========================================================
typedef enum
{
	STA, AP, STA_AP
} ENUM_Net_ModeTypeDef;

typedef enum
{
	enumTCP, enumUDP
} ENUM_NetPro_TypeDef;
typedef enum
{
	Multiple_ID_0 = 0,
	Multiple_ID_1 = 1,
	Multiple_ID_2 = 2,
	Multiple_ID_3 = 3,
	Multiple_ID_4 = 4,
	Single_ID_0 = 5
} ENUM_ID_NO_TypeDef;
typedef enum
{
	In4G = 1, InWifi = 2, InUsart1 = 3
} ENUM_Internet_TypeDef;
typedef enum
{
	DOWN_IAP_RegiseterErro = 0x01,
	DOWN_IAP_RegiButUpdate = 0x02,
	DOWN_IAP_RegiseterSucc = 0x03,
	DOWN_IAP_CurrentVersion = 0x05,
	DOWN_IAP_N_Frame = 0x07,
	DOWN_IAP_RecivedUpdateComp = 0x09,
	DOWN_IAP_RecivedErrRep = 0xE1
} ENUM_tcpDOWN_TypeDef;

typedef enum
{
	UP_IAP_Regiser = 0x00,
	UP_IAP_CurrentVersion = 0x04,
	UP_IAP_N_Frame = 0x06,
	UP_IAP_UpdateComp = 0x08,
	UP_IAP_ErrorReport = 0xE0
} ENUM_tcpUP_TypeDef;

void _USART_Printf(ENUM_Internet_TypeDef net, const char *fmt, ...);
bool Send_AT_Cmd(ENUM_Internet_TypeDef internet, char *cmd, char *ack1,
		char *ack2, u32 time, u16 retry, FunctionalState printState);
bool AT_Test(ENUM_Internet_TypeDef internet);
void DEBUG(const char *fmt, ...);

void IAP_getRegisterStr(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD,
		char moduleType);
void IAP_getUpdateStr(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD, u16 num);
void IAP_getUpdateComplStr(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD);
void IAP_request4Register(ENUM_Internet_TypeDef internet);
void IAP_request4UpdateComp(ENUM_Internet_TypeDef internet);
void IAP_request4Update(ENUM_Internet_TypeDef internet);
void IAP_request4_NFrame(ENUM_Internet_TypeDef internet, u16 num);
void IAP_request4Error(ENUM_Internet_TypeDef internet);
bool IAP_analyse(ENUM_Internet_TypeDef internet, u8 *cmd, char *buf, u16 bufLen,
		u16 timeout);
void IAP_process(ENUM_Internet_TypeDef internet, ENUM_tcpDOWN_TypeDef DownCMD,
		char *SData);

#endif /* _TCP_PUBLIC_H_ */
