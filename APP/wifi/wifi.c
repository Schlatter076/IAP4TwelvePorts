/*
 * wifi.c
 *
 *  Created on: 2021年1月8日
 *      Author: HHS007
 */
#include "wifi.h"

void USART2_Init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	USART_DeInit(USART2);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	//=====================================================================================================
	DMA_USART_Tx_Init(USART2, RCC_AHB1Periph_DMA1, DMA1_Stream6_IRQn, 0, 1,
	DMA1_Stream6, DMA_Channel_4, (uint32_t) (&USART2->DR),
			(uint32_t) WIFI_Fram.TxBuf, BASE64_BUF_LEN, DMA_Priority_High);

	DMA_USART_Rx_Init(USART2, RCC_AHB1Periph_DMA1, DMA1_Stream5, DMA_Channel_4,
			(uint32_t) (&USART2->DR), (uint32_t) WIFI_Fram.RxBuf,
			TCP_MAX_LEN, DMA_Priority_VeryHigh);
	//=====================================================================================================
	USART_InitStructure.USART_BaudRate = bound; //波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No; //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl =
	USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART2, &USART_InitStructure); //初始化串口

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);  //根据指定的参数初始化VIC寄存器

	//中断配置
	USART_ITConfig(USART2, USART_IT_TC, DISABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
	//启动串口
	USART_Cmd(USART2, ENABLE);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //使能GPIO时钟
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void WIFI_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART2_Init(bound);
}

/**
 * 选择wifi模块的工作模式
 * @enumMode：工作模式
 * @return 返回1：选择成功 0：选择失败
 */
bool WIFI_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode)
{
	switch (enumMode)
	{
	case STA:
		return Send_AT_Cmd(InWifi, "AT+CWMODE_CUR=1", "OK", "no change", 1800,
				2, ENABLE);

	case AP:
		return Send_AT_Cmd(InWifi, "AT+CWMODE_CUR=2", "OK", "no change", 1800,
				2, ENABLE);

	case STA_AP:
		return Send_AT_Cmd(InWifi, "AT+CWMODE_CUR", "OK", "no change", 1800, 2,
				ENABLE);

	default:
		return false;
	}
}

/**
 * wifi模块连接外部WiFi
 * @pSSID：WiFi名称字符串
 * @pPassWord：WiFi密码字符串
 * @return 返回1：连接成功 0：连接失败
 */
bool WIFI_JoinAP(char * pSSID, char * pPassWord)
{
	char *cCmd;
	bool ret = false;
	cCmd = mymalloc(120);
	snprintf(cCmd, 120, "AT+CWJAP_CUR=\"%s\",\"%s\"", pSSID, pPassWord);
	ret = Send_AT_Cmd(InWifi, cCmd, "OK", NULL, 1800, 2, ENABLE);
	myfree(cCmd);
	return ret;
}

/**
 * wifi模块启动多连接
 * @enumEnUnvarnishTx：配置是否多连接
 * @return 1：配置成功 0：配置失败
 */
bool WIFI_Enable_MultipleId(FunctionalState enumEnUnvarnishTx)
{
	char *cStr;
	bool ret = false;
	cStr = mymalloc(20);
	snprintf(cStr, 20, "AT+CIPMUX=%d", (enumEnUnvarnishTx ? 1 : 0));
	ret = Send_AT_Cmd(InWifi, cStr, "OK", 0, 500, 2, ENABLE);
	return ret;
}

/**
 * wifi模块连接外部服务器
 * @enumE：网络协议
 * @ip：服务器IP字符串
 * @ComNum：服务器端口字符串
 * @id：模块连接服务器的ID
 * @return 1：连接成功 0：连接失败
 */
bool WIFI_Link_Server(ENUM_NetPro_TypeDef enumE, char * ip, char* ComNum,
		ENUM_ID_NO_TypeDef id)
{
	char *cStr = mymalloc(100);
	char *cCmd = mymalloc(120);
	bool rc = false;

	switch (enumE)
	{
	case enumTCP:
		sprintf(cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum);
		break;

	case enumUDP:
		sprintf(cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum);
		break;

	default:
		break;
	}

	if (id < 5)
		sprintf(cCmd, "AT+CIPSTART=%d,%s", id, cStr);

	else
		sprintf(cCmd, "AT+CIPSTART=%s", cStr);

	rc = Send_AT_Cmd(InWifi, cCmd, "OK", "ALREADY CONNECT", 1800, 2, ENABLE);
	myfree(cStr);
	myfree(cCmd);
	return rc;
}

/**
 * 配置wifi模块进入透传发送
 * @return 1：配置成功 0：配置失败
 */
bool WIFI_UnvarnishSend(void)
{
	if (!Send_AT_Cmd(InWifi, "AT+CIPMODE=1", "OK", 0, 500, 2, ENABLE))
		return false;

	return Send_AT_Cmd(InWifi, "AT+CIPSEND", "OK", ">", 500, 2, ENABLE);

}

/**
 * wifi模块发送字符串
 * @enumEnUnvarnishTx：声明是否已使能了透传模式
 * @pStr：要发送的字符串
 * @ulStrLength：要发送的字符串的字节数
 * @ucId：哪个ID发送的字符串
 * @return 1：发送成功 0：发送失败
 */
//bool WIFI_SendString(FunctionalState enumEnUnvarnishTx, char * pStr,
//		u32 ulStrLength, ENUM_ID_NO_TypeDef ucId)
//{
//	char cStr[20];
//	bool bRet = false;
//
//	if (enumEnUnvarnishTx)
//	{
//		_USART_Printf(InWifi, "%s", pStr);
//		bRet = true;
//	}
//	else
//	{
//		if (ucId < 5)
//			sprintf(cStr, "AT+CIPSENDEX=%d,%d", ucId,
//					(unsigned int) ulStrLength + 2);
//
//		else
//			sprintf(cStr, "AT+CIPSENDEX=%d", (unsigned int) ulStrLength + 2);
//
//		Send_AT_Cmd(InWifi, cStr, "> ", 0, 1000, 2);
//
//		bRet = Send_AT_Cmd(InWifi, pStr, "SEND OK", 0, 1000, 2);
//	}
//	return bRet;
//}
/**
 * WIFI发送字符串
 * @data 待发送的字符串
 */
void WIFI_Send(const char *data)
{
	char *p_str;
	char *buf = mymalloc(20);
	p_str = mymalloc(BASE64_BUF_LEN);
	memset(buf, '\0', 20);
	memset(p_str, '\0', BASE64_BUF_LEN);
	base64_encode((const unsigned char *) data, p_str);
	snprintf(buf, 20, "AT+CIPSENDEX=%d", strlen((const char *) p_str) + 3);
	if (Send_AT_Cmd(InWifi, buf, "> ", NULL, 200, 2, DISABLE))
	{
		_USART_Printf(InWifi, "{(%s}", p_str);
		DEBUG("wifi<<%s\r\n", data);
	}
	myfree(p_str);
	myfree(buf);
}

//wifi模块退出透传模式
void WIFI_ExitUnvarnishSend(void)
{
	delay_ms(1000);
	_USART_Printf(InWifi, "+++");
	delay_ms(500);
}

/**
 * wifi 的连接状态，较适合单端口时使用
 * 返回0：获取状态失败
 * 返回2：获得ip
 * 返回3：建立连接
 * 返回4：失去连接
 */
u8 WIFI_Get_LinkStatus(void)
{
	if (Send_AT_Cmd(InWifi, "AT+CIPSTATUS", "OK", 0, 500, 2, DISABLE))
	{
		if (strstr((const char *) WIFI_Fram.RxBuf, "STATUS:2\r\n"))
			return 2;

		else if (strstr((const char *) WIFI_Fram.RxBuf, "STATUS:3\r\n"))
			return 3;

		else if (strstr((const char *) WIFI_Fram.RxBuf, "STATUS:4\r\n"))
			return 4;
	}
	return 0;
}

bool ConnectToServerByWIFI(char* addr, char* port)
{
	u8 cnt = 0;
	WIFI_Fram.allowHeart = 0;
	WIFI_Net_Mode_Choose(STA);
	//Send_AT_Cmd(InWifi, "AT+GMR", "OK", NULL, 500, 2);
	while (cnt < 8)
	{
		cnt++;
		if (WIFI_JoinAP((char *) MyFlashParams.WifiSSID,
				(char *) MyFlashParams.WifiPWD))
		{
			break;
		}
	}
	if (cnt < 8)
	{
		WIFI_Enable_MultipleId(DISABLE);
		if (WIFI_Link_Server(enumTCP, addr, port, Single_ID_0))
		{
			return true;
		}
	}
	return false;
}
/**
 * wifi上电
 * @return 0-失败  1-succ
 */
u8 Wifi_PowerOn(void)
{
	//1.检查是否配置过wifi
	if (MyFlashParams.WifiFlag == WIFI_FLAG)
	{
		//2.检查模块是否需要重新开关机
		WIFI_Fram.AT_test_OK = AT_Test(InWifi);
		if (!WIFI_Fram.AT_test_OK)
		{
			//3.执行开关机操作
			_WIFI_RST = 0;
			delay_ms(1000);
			delay_ms(1000);
			_WIFI_RST = 1;
			delay_ms(100);
			//4.再次检查模块是否正常工作
			WIFI_Fram.AT_test_OK = AT_Test(InWifi);
		}
		//5.如果模块已经正常工作
		if (WIFI_Fram.AT_test_OK)
		{
			//6.执行TCP连接
			WIFI_Fram.Online = ConnectToServerByWIFI(TCP_IP, TCP_PORT);
			return WIFI_Fram.Online;
		}
	}
	return 0;
}

void DMA1_Stream6_IRQHandler(void)
{
#if SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif
	if (DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6) != RESET)
	{
		//清除标志位
		DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);
		//关闭DMA
		DMA_Cmd(DMA1_Stream6, DISABLE);
		//打开发送完成中断,发送最后两个字节
		USART_ITConfig(USART2, USART_IT_TC, ENABLE);
	}
#if SYSTEM_SUPPORT_OS
	OSIntExit(); //退出中断
#endif
}

void USART2_IRQHandler(void)
{
#if SYSTEM_SUPPORT_OS  //使用UCOS操作系统
	OS_ERR err;
	OSIntEnter();
#endif
	if (USART_GetITStatus(USART2, USART_IT_TC) != RESET)
	{
		//关闭发送完成中断
		USART_ITConfig(USART2, USART_IT_TC, DISABLE);
		//发送完成
		WIFI_Fram.DMA_Tx_Busy = 0;
#if SYSTEM_SUPPORT_OS
		//推送发送完成
		OSFlagPost((OS_FLAG_GRP*) &EventFlags,//对应的事件标志组
				(OS_FLAGS) FLAG_USART2_TxED,//事件位
				(OS_OPT) OS_OPT_POST_FLAG_SET,//选择置位
				(OS_ERR*) &err);//错误码
#endif
	}
	if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		USART2->SR; //先读SR，再读DR
		USART2->DR;

		//关闭DMA
		DMA_Cmd(DMA1_Stream5, DISABLE);
		//清除标志位
		DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5);
		//获得接收帧帧长
		WIFI_Fram.AccessLen = TCP_MAX_LEN
				- DMA_GetCurrDataCounter(DMA1_Stream5);
		WIFI_Fram.RxBuf[WIFI_Fram.AccessLen] = '\0'; //添加结束符

		WIFI_Fram.FinishFlag = 1;
		//这里可以通知任务来处理数据
#if SYSTEM_SUPPORT_OS
		//推送接收完成
		OSFlagPost((OS_FLAG_GRP*) &EventFlags,//对应的事件标志组
				(OS_FLAGS) FLAG_USART2_RxED,//事件位
				(OS_OPT) OS_OPT_POST_FLAG_SET,//选择置位
				(OS_ERR*) &err);//错误码
#endif
		//设置传输数据长度
		DMA_SetCurrDataCounter(DMA1_Stream5, TCP_MAX_LEN);
		//打开DMA
		DMA_Cmd(DMA1_Stream5, ENABLE);
	}
#if SYSTEM_SUPPORT_OS
	OSIntExit();    	//退出中断
#endif
}
