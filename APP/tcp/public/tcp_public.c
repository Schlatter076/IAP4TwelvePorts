/*
 * tcp_public.c
 *
 *  Created on: 2021年1月5日
 *      Author: HHS007
 */
#include "tcp_public.h"

struct STRUCT_USART_Fram F4G_Fram =
{ 0 };
struct STRUCT_USART_Fram WIFI_Fram =
{ 0 };
struct STRUCT_USART_Fram USART1_Fram =
{ 0 };
struct STRUCT_USART_Params TCP_Params =
{ 0 };

/**
 * 串口格式化输出数据
 * @net 选择端口
 * @fmt 格式化参数
 * @return 无
 */
void _USART_Printf(ENUM_Internet_TypeDef net, const char *fmt, ...)
{
	va_list ap;
	struct STRUCT_USART_Fram *fram;
	DMA_Stream_TypeDef* DMAy_Streamx;
	if (net == In4G)
	{
		fram = &F4G_Fram;
		DMAy_Streamx = DMA1_Stream3;
	}
	else if (net == InWifi)
	{
		fram = &WIFI_Fram;
		DMAy_Streamx = DMA1_Stream6;
	}
	else
	{
		fram = &USART1_Fram;
		DMAy_Streamx = DMA2_Stream7;
	}
	//等待空闲,防止数据不同步
	while (fram->DMA_Tx_Busy)
		;
	fram->DMA_Tx_Busy = 1;
	va_start(ap, fmt);
	vsprintf((char *) fram->TxBuf, fmt, ap);
	va_end(ap);
	//设置传输数据长度
	DMA_SetCurrDataCounter(DMAy_Streamx, strlen((const char *) fram->TxBuf));
	//打开DMA,开始发送
	DMA_Cmd(DMAy_Streamx, ENABLE);
}
/**
 * 串口1调试打印
 * DMA方式发送
 */
void DEBUG(const char *fmt, ...)
{
	va_list ap;
	//等待空闲,防止数据不同步
	while (USART1_Fram.DMA_Tx_Busy)
		;
	USART1_Fram.DMA_Tx_Busy = 1;
	va_start(ap, fmt);
	vsprintf((char *) USART1_Fram.TxBuf, fmt, ap);
	va_end(ap);
	//设置传输数据长度
	DMA_SetCurrDataCounter(DMA2_Stream7,
			strlen((const char *) USART1_Fram.TxBuf));
	//打开DMA,开始发送
	DMA_Cmd(DMA2_Stream7, ENABLE);
}

/**
 * 对模块发送AT指令
 * @cmd：待发送的指令
 * @ack1，@ack2：期待的响应，为NULL表不需响应，两者为或逻辑关系
 * @time：等待响应的时间(时间片长度)
 * @retry 重试次数
 * @return 1：发送成功 0：失败
 */
bool Send_AT_Cmd(ENUM_Internet_TypeDef internet, char *cmd, char *ack1,
		char *ack2, u32 time, u16 retry)
{
	struct STRUCT_USART_Fram *USART_Fram = NULL;
	bool ret = false;
	u16 cnt = 0;

	if (internet == In4G)
	{
		USART_Fram = &F4G_Fram;
	}
	else if (internet == InWifi)
	{
		USART_Fram = &WIFI_Fram;
	}
	_USART_Printf(internet, "%s\r\n", cmd); //发送指令
	if (ack1 == 0 && ack2 == 0)	 //不需要接收数据
	{
		return true;
	}
	while (ret == false && cnt++ < retry)
	{

		//调试打印
		DEBUG("%s\r\n", USART_Fram->RxBuf);
		if (ack1 != 0 && ack2 != 0)
		{
			if (strstr((const char *) USART_Fram->RxBuf, ack1)
					|| strstr((const char *) USART_Fram->RxBuf, ack2))
			{
				ret = true;
			}
		}
		else if (ack1 != 0)
		{
			if (strstr((const char *) USART_Fram->RxBuf, ack1))
			{
				ret = true;
			}
		}
		else if (ack2 != 0)
		{
			if (strstr((const char *) USART_Fram->RxBuf, ack2))
			{
				ret = true;
			}
		}
		else if (strstr((const char *) USART_Fram->RxBuf, "ERROR"))
		{
			ret = false;
		}
	}
	return ret;
}

bool AT_Test(ENUM_Internet_TypeDef internet)
{
	u8 count = 0;
	char *module = mymalloc(10);
	if (internet == In4G)
	{
		sprintf(module, "%s", "4G module");
	}
	else
	{
		sprintf(module, "%s", "WIFI module");
	}
	while (count++ < 8)
	{
		if (Send_AT_Cmd(internet, "AT", "OK", NULL, 100, 2))
		{
			DEBUG("test %s success!\r\n", module);
			myfree(module);
			return 1;
		}
	}
	DEBUG("test %s fail!\r\n", module);
	myfree(module);
	return 0;
}

/**
 * 获取注册字符串
 * @strBuf 待获取的字符串
 * @len 字符串长度
 * @upCMD 上行控制字
 * @modulType 所使用的的模块类型
 * 无返回
 */
void getRegisterStr(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD,
		char moduleType)
{
	const char* template = "%s,%d,%s-%c-%c-%s-%s_%s-%s";
	snprintf(strBuf, len, template, MyFlashParams.DeviceID, upCMD,
			TCP_Params.ccid, TCP_Params.cops, moduleType, MyFlashParams.Version,
			"0", "0", "12");
}
/**
 * 获取不带参数的请求字符串
 * @strBuf
 * @len
 * @upCMD 上行控制字
 * 无返回
 */
void getRequestStrWithoutParam(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD)
{
	const char* template = "%s,%d";
	snprintf(strBuf, len, template, MyFlashParams.DeviceID, upCMD);
}

/**
 * 请求注册
 * @internet 端口
 */
void request4Register(ENUM_Internet_TypeDef internet)
{
	char *buf = mymalloc(100);
	memset(buf, '\0', 100);
	getRegisterStr(buf, 100, UP_Regiser, '2');
	if (internet == In4G)
	{
		Module4G_Send(buf);
	}
	else if (internet == InWifi)
	{
		WIFI_Send(buf);
	}
	myfree(buf);
}
