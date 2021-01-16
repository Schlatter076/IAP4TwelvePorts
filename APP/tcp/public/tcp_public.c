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
		char *ack2, u32 time, u16 retry, FunctionalState printState)
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
		for (int i = 0; i < time; i += 20)
		{
			delay_ms(100);
			if (USART_Fram->FinishFlag)
			{
				USART_Fram->FinishFlag = 0;
				//调试打印
				if (printState)
				{
					DEBUG("%s\r\n", USART_Fram->RxBuf);
				}
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
				break;
			}
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
		Send_AT_Cmd(internet, "AT", "OK", NULL, 100, 1, DISABLE);
	}
	if (Send_AT_Cmd(internet, "AT", "OK", NULL, 100, 1, DISABLE))
	{
		DEBUG("test %s success!\r\n", module);
		myfree(module);
		return 1;
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
void IAP_getRegisterStr(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD,
		char moduleType)
{
	const char *template = "%s,%02X,%s-%c-%c-%s-%s_%s-%s";
	snprintf(strBuf, len, template, MyFlashParams.DeviceID, upCMD,
			MyFlashParams.ccid, MyFlashParams.cops, moduleType,
			MyFlashParams.Version, "0", "0", "12");
}
/**
 * 获取更新字符串
 * @strBuf
 * @len
 * @upCMD
 * @num 帧数
 */
void IAP_getUpdateStr(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD, u16 num)
{
	const char* template = "%s,%02X,%d";
	snprintf(strBuf, len, template, MyFlashParams.DeviceID, upCMD, num);
}
/**
 * 获取更新完成字符串
 * @strBuf
 * @len
 * @upCMD
 * @num 帧数
 */
void IAP_getUpdateComplStr(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD)
{
	const char* template = "%s,%02X,%s";
	snprintf(strBuf, len, template, MyFlashParams.DeviceID, upCMD,
			MyFlashParams.Version);
}

/**
 * 请求注册
 * @internet 端口
 */
void IAP_request4Register(ENUM_Internet_TypeDef internet)
{
	char *buf = mymalloc(100);
	memset(buf, '\0', 100);
	IAP_getRegisterStr(buf, 100, UP_IAP_Regiser, '2');
	if (internet == In4G)
	{
		Module4G_Send(buf);
	}
	else if (internet == InWifi)
	{
		WIFI_Send(buf);
	}
	else
	{
		DEBUG("{%s}\r\n", buf);
	}
	myfree(buf);
}
/**
 * 请求更新完成
 */
void IAP_request4UpdateComp(ENUM_Internet_TypeDef internet)
{
	char *buf = mymalloc(100);
	memset(buf, '\0', 100);
	IAP_getUpdateComplStr(buf, 100, UP_IAP_UpdateComp);
	if (internet == In4G)
	{
		Module4G_Send(buf);
	}
	else if (internet == InWifi)
	{
		WIFI_Send(buf);
	}
	else
	{
		DEBUG("{%s}\r\n", buf);
	}
	myfree(buf);
}
/**
 * 请求更新
 */
void IAP_request4Update(ENUM_Internet_TypeDef internet)
{
	char *buf = mymalloc(100);
	memset(buf, '\0', 100);
	IAP_getRegisterStr(buf, 100, UP_IAP_CurrentVersion, '2');
	if (internet == In4G)
	{
		Module4G_Send(buf);
	}
	else if (internet == InWifi)
	{
		WIFI_Send(buf);
	}
	else
	{
		DEBUG("{%s}\r\n", buf);
	}
	myfree(buf);
}
/**
 * 请求第N帧
 */
void IAP_request4_NFrame(ENUM_Internet_TypeDef internet, u16 num)
{
	char *buf = mymalloc(100);
	memset(buf, '\0', 100);
	IAP_getUpdateStr(buf, 100, UP_IAP_N_Frame, num);
	if (internet == In4G)
	{
		Module4G_Send(buf);
	}
	else if (internet == InWifi)
	{
		WIFI_Send(buf);
	}
	else
	{
		DEBUG("{%s}\r\n", buf);
	}
	myfree(buf);
}
/**
 * 请求Error
 */
void IAP_request4Error(ENUM_Internet_TypeDef internet)
{
	const char *template = "%s,%02X";
	char *buf = mymalloc(100);
	memset(buf, '\0', 100);
	snprintf(buf, 100, template, MyFlashParams.DeviceID, UP_IAP_ErrorReport);
	if (internet == In4G)
	{
		Module4G_Send(buf);
	}
	else if (internet == InWifi)
	{
		WIFI_Send(buf);
	}
	else
	{
		DEBUG("{%s}\r\n", buf);
	}
	myfree(buf);
}
/**
 * 解析服务器下发的数据
 * @internet 端口
 * @cmd 解析后的命令
 * @buf 获取解析后的字符串
 * @bufLen 长度
 * @timeout 超时时间 - 5ms为一个单位
 * @return true-成功解析  false-超时时间内未收到正确的数据
 */
bool IAP_analyse(ENUM_Internet_TypeDef internet, u8 *cmd, char *buf, u16 bufLen,
		u16 timeout)
{
	char *res = NULL;
	bool needDecrypt = false;
	char *DecryptionBuf = NULL;
	struct STRUCT_USART_Fram *fram = NULL;
	if (internet == In4G)
	{
		fram = &F4G_Fram;
		needDecrypt = true;
	}
	else if (internet == InWifi)
	{
		fram = &WIFI_Fram;
		needDecrypt = true;
	}
	else
	{
		fram = &USART1_Fram;
		needDecrypt = false;
	}
	memset(buf, '\0', bufLen);
	while (timeout--)
	{
		delay_ms(5);
		if (fram->FinishFlag)
		{
			fram->FinishFlag = 0;
			res = strchr((const char *) fram->RxBuf, '[');
			if (res && strchr((const char *) fram->RxBuf, ']'))
			{
				res += 1; //跳过字符'['
				if (strchr(res, '[')) //判断是否还有字符'[',防止数据出错
				{
					res = strchr(res, '[');
					res += 1; //跳过字符'['
				}
				if (strlen(res) > 8)
				{
					DecryptionBuf = mymalloc(BASE64_BUF_LEN);
					memset(DecryptionBuf, '\0', BASE64_BUF_LEN);
					res = strtok(res, "]"); //取出实际的数据
					if (needDecrypt)
					{
						needDecrypt = false;
						base64_decode(res, (unsigned char *) DecryptionBuf);
					}
					else
					{
						strcpy(DecryptionBuf, res);
					}
					DEBUG(">>%s\r\n", DecryptionBuf);
					res = strchr(DecryptionBuf, ','); //跳过设备编号
					res += 1;  //跳过字符','
					//获取指令
					*cmd = strtol(res, NULL, 16); //取出指令
					//找到第二个逗号
					res = strchr(res, ',');
					if(res)
					{
						res += 1;
						if(res)
						{
							//如果有数据，复制
							strcpy(buf, res);
						}
					}
					myfree(DecryptionBuf);  //释放临时资源
					return true;
				}
				break;
			}
		}
	}
	return false;
}
/**
 * 处理服务端指令
 * @intenet 4G 或 Wifi
 * @DownCMD 下行指令
 * @SData 携带的信息体
 * 无返回
 */
void IAP_process(ENUM_Internet_TypeDef internet, ENUM_tcpDOWN_TypeDef DownCMD,
		char *SData)
{
	switch (DownCMD)
	{
	case DOWN_IAP_RegiseterSucc:
		DEBUG("Server Params=%s\r\n", SData);
		strcpy((char *)MyFlashParams.ServerParams, SData);
		MyFlashParams.ServerParams[100] = '\0';
		MyFlashParams.IAPFlag = IAP_APPRUN_FLAG_DATA;
		break;
	case DOWN_IAP_RegiseterErro:
		MyFlashParams.IAPFlag = IAP_INIT_FLAG_DATA;
		break;
	case DOWN_IAP_RegiButUpdate:
		MyFlashParams.IAPFlag = IAP_UPDATE_FLAG_DATA;
		break;
	case DOWN_IAP_CurrentVersion:

		break;
	case DOWN_IAP_N_Frame:

		break;
	case DOWN_IAP_RecivedUpdateComp:
		DEBUG("update data checked success!\r\n");
		break;
	case DOWN_IAP_RecivedErrRep:
		MyFlashParams.IAPFlag = IAP_INIT_FLAG_DATA;
		break;
	default:
		DEBUG("cmd\"%d\" is not support\r\n", DownCMD);
		break;
	}
}
