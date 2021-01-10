/*
 * F4G.c
 *
 *  Created on: 2021年1月8日
 *      Author: HHS007
 */
#include "F4G.h"

void USART3_Init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	USART_DeInit(USART3);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	//=====================================================================================================
	DMA_USART_Tx_Init(USART3, RCC_AHB1Periph_DMA1, DMA1_Stream3_IRQn, 1, 1,
	DMA1_Stream3, DMA_Channel_4, (uint32_t) (&USART3->DR),
			(uint32_t) F4G_Fram.TxBuf, BASE64_BUF_LEN, DMA_Priority_High);

	DMA_USART_Rx_Init(USART3, RCC_AHB1Periph_DMA1, DMA1_Stream1, DMA_Channel_4,
			(uint32_t) (&USART3->DR), (uint32_t) F4G_Fram.RxBuf,
			TCP_MAX_LEN, DMA_Priority_VeryHigh);
	//=====================================================================================================
	USART_InitStructure.USART_BaudRate = bound; //波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No; //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl =
	USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART3, &USART_InitStructure); //初始化串口

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);  //根据指定的参数初始化VIC寄存器

	//中断配置
	USART_ITConfig(USART3, USART_IT_TC, DISABLE);
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
	//启动串口
	USART_Cmd(USART3, ENABLE);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); //使能GPIO时钟
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void F4G_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	USART3_Init(bound);
}

/**
 * 通过4G网络连接到服务器
 * @addr IP地址或域名
 * @port 端口
 * @return
 */
bool ConnectToServerBy4G(char* addr, char* port)
{
	bool res = false;
	F4G_Fram.allowHeart = 0;
	char *p = mymalloc(100);
	sprintf(p, "AT+CIPSTART=\"TCP\",\"%s\",%s", addr, port);
	Send_AT_Cmd(In4G, "AT+CIPSHUT", "SHUT OK", NULL, 100, 2);
	Send_AT_Cmd(In4G, "AT+CREG?", "OK", NULL, 100, 2);
	Send_AT_Cmd(In4G, "AT+CGATT?", "OK", NULL, 100, 2);
	//单链接
	Send_AT_Cmd(In4G, "AT+CIPMUX=0", "OK", NULL, 100, 2);
	//快传
	Send_AT_Cmd(In4G, "AT+CIPQSEND=1", "OK", NULL, 100, 2);
	if (TCP_Params.cops == '3')
	{
		Send_AT_Cmd(In4G, "AT+CSTT=cmiot", "OK", NULL, 360, 2);
	}
	else if (TCP_Params.cops == '6')
	{
		Send_AT_Cmd(In4G, "AT+CSTT=UNIM2M.NJM2MAPN", "OK", NULL, 360, 2);
	}
	else if (TCP_Params.cops == '9')
	{
		Send_AT_Cmd(In4G, "AT+CSTT=CTNET", "OK", NULL, 360, 2);
	}
	Send_AT_Cmd(In4G, "AT+CIICR", "OK", NULL, 100, 2);
	Send_AT_Cmd(In4G, "AT+CIFSR", "OK", NULL, 100, 2);
	Send_AT_Cmd(In4G, p, "CONNECT", NULL, 360, 2);
	res = Send_AT_Cmd(In4G, "AT+CIPSTATUS", "CONNECT OK", NULL, 360, 2);
	myfree(p);
	return res;
}
/***********************以下开始为与服务器通信业务代码部分*************************************/
void getModuleMes(void)
{
	unsigned char *result = NULL;
	u8 inx = 0;
	//获取物联网卡号
	while (!Send_AT_Cmd(In4G, "AT+ICCID", "+ICCID:", NULL, 100, 2))
		;
	result = F4G_Fram.RxBuf;
	inx = 0;
	while (!(*result <= '9' && *result >= '0'))
	{
		result++;
	}
	//当值为字母和数字时
	while ((*result <= '9' && *result >= '0')
			|| (*result <= 'Z' && *result >= 'A')
			|| (*result <= 'z' && *result >= 'a'))
	{
		TCP_Params.ccid[inx++] = *result;
		result++;
	}
	DEBUG("CCID=%s\r\n", TCP_Params.ccid);

	//获取模块网络信息
	while (!Send_AT_Cmd(In4G, "AT+COPS=0,1", "OK", NULL, 200, 2))
		;
	while (!Send_AT_Cmd(In4G, "AT+COPS?", "+COPS", NULL, 100, 2))
		;
	if ((bool) strstr((const char *) F4G_Fram.RxBuf, "CMCC"))
	{
		TCP_Params.cops = '3';
	}
	else if ((bool) strstr((const char *) F4G_Fram.RxBuf, "UNICOM"))
	{
		TCP_Params.cops = '6';
	}
	else
	{
		TCP_Params.cops = '9';
	}
	DEBUG("COPS is \"%c\"\r\n", TCP_Params.cops);
	//获取信号
	while (!Send_AT_Cmd(In4G, "AT+CSQ", "+CSQ", NULL, 100, 2))
		;
	result = F4G_Fram.RxBuf;
	while (*result++ != ':')
		;
	result++;
	TCP_Params.rssi = atoi(strtok((char *) result, ","));
	DEBUG("CSQ is %d\r\n", TCP_Params.rssi);
}

/**
 * 通过4G网络发送数据
 * @data 待发送的数据
 * 无返回
 */
void Module4G_Send(const char *data)
{
	char *p_str;
	char *buf = mymalloc(20);
	p_str = mymalloc(BASE64_BUF_LEN);
	base64_encode((const unsigned char *)data, p_str);
	snprintf(buf, 20, "AT+CIPSEND=%d", strlen((const char *) p_str) + 3);
	if (Send_AT_Cmd(In4G, buf, ">", NULL, 200, 2))
	{
		_USART_Printf(In4G, "{(%s}", p_str);
		DEBUG("<<%s", data);
	}
	myfree(p_str);
	myfree(buf);
}

void DMA1_Stream3_IRQHandler(void)
{
#if SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif
	if (DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3) != RESET)
	{
		//清除标志位
		DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);
		//关闭DMA
		DMA_Cmd(DMA1_Stream3, DISABLE);
		//打开发送完成中断,发送最后两个字节
		USART_ITConfig(USART3, USART_IT_TC, ENABLE);
	}
#if SYSTEM_SUPPORT_OS
	OSIntExit(); //退出中断
#endif
}

void USART3_IRQHandler(void)
{
#if SYSTEM_SUPPORT_OS  //使用UCOS操作系统
	OS_ERR err;
	OSIntEnter();
#endif
	if (USART_GetITStatus(USART3, USART_IT_TC) != RESET)
	{
		//关闭发送完成中断
		USART_ITConfig(USART3, USART_IT_TC, DISABLE);
		//发送完成
		F4G_Fram.DMA_Tx_Busy = 0;
#if SYSTEM_SUPPORT_OS
		//推送发送完成
		OSFlagPost((OS_FLAG_GRP*) &EventFlags, //对应的事件标志组
				(OS_FLAGS) FLAG_USART3_TxED, //事件位
				(OS_OPT) OS_OPT_POST_FLAG_SET, //选择置位
				(OS_ERR*) &err); //错误码
#endif
	}
	if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		USART3->SR; //先读SR，再读DR
		USART3->DR;

		F4G_Fram.InfBit.FinishFlag = 1; //接收到一帧数据

		//关闭DMA
		DMA_Cmd(DMA1_Stream1, DISABLE);
		//清除标志位
		DMA_ClearFlag(DMA1_Stream1, DMA_FLAG_TCIF1);
		//获得接收帧帧长
		F4G_Fram.AccessLen = TCP_MAX_LEN - DMA_GetCurrDataCounter(DMA1_Stream1);
		//这里可以通知任务来处理数据
#if SYSTEM_SUPPORT_OS
		//推送接收完成
		OSFlagPost((OS_FLAG_GRP*) &EventFlags, //对应的事件标志组
				(OS_FLAGS) FLAG_USART3_RxED, //事件位
				(OS_OPT) OS_OPT_POST_FLAG_SET, //选择置位
				(OS_ERR*) &err); //错误码
#endif
		//设置传输数据长度
		DMA_SetCurrDataCounter(DMA1_Stream1, TCP_MAX_LEN);
		//打开DMA
		DMA_Cmd(DMA1_Stream1, ENABLE);
	}
#if SYSTEM_SUPPORT_OS
	OSIntExit();    	//退出中断
#endif
}

