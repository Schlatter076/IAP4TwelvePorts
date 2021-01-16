/*
 * usart.c
 *
 *  Created on: 2021年1月5日
 *      Author: HHS007
 */
#include "usart.h"

bool localUpdate = false;

void USART1_Init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	USART_DeInit(USART1);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); //使能USART6时钟
	//=====================================================================================================
	DMA_USART_Tx_Init(USART1, RCC_AHB1Periph_DMA2, DMA2_Stream7_IRQn, 3, 1,
	DMA2_Stream7, DMA_Channel_4, (uint32_t) (&USART1->DR),
			(uint32_t) USART1_Fram.TxBuf, BASE64_BUF_LEN, DMA_Priority_Medium);

	DMA_USART_Rx_Init(USART1, RCC_AHB1Periph_DMA2, DMA2_Stream2, DMA_Channel_4,
			(uint32_t) (&USART1->DR), (uint32_t) USART1_Fram.RxBuf,
			TCP_MAX_LEN, DMA_Priority_High);
	//=====================================================================================================
	USART_InitStructure.USART_BaudRate = bound; //波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No; //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl =
	USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART1, &USART_InitStructure); //初始化串口

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);  //根据指定的参数初始化VIC寄存器

	//中断配置
	USART_ITConfig(USART1, USART_IT_TC, DISABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
	//启动串口
	USART_Cmd(USART1, ENABLE);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //使能GPIO时钟
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void DMA2_Stream7_IRQHandler(void)
{
#if SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif
	if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) != RESET)
	{
		//清除标志位
		DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7);
		//关闭DMA
		DMA_Cmd(DMA2_Stream7, DISABLE);
		//打开发送完成中断,发送最后两个字节
		USART_ITConfig(USART1, USART_IT_TC, ENABLE);
	}
#if SYSTEM_SUPPORT_OS
	OSIntExit(); //退出中断
#endif
}

void USART1_IRQHandler(void)                	//串口1中断服务程序
{
#if SYSTEM_SUPPORT_OS  //使用UCOS操作系统
	OS_ERR err;
	OSIntEnter();
#endif
	if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
	{
		//关闭发送完成中断
		USART_ITConfig(USART1, USART_IT_TC, DISABLE);
		//发送完成
		USART1_Fram.DMA_Tx_Busy = 0;
#if SYSTEM_SUPPORT_OS
		//推送发送完成
		OSFlagPost((OS_FLAG_GRP*) &EventFlags,//对应的事件标志组
				(OS_FLAGS) FLAG_USART1_TxED,//事件位
				(OS_OPT) OS_OPT_POST_FLAG_SET,//选择置位
				(OS_ERR*) &err);//错误码
#endif
	}
	if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		USART1->SR; //先读SR，再读DR
		USART1->DR;

		//关闭DMA
		DMA_Cmd(DMA2_Stream2, DISABLE);
		//清除标志位
		DMA_ClearFlag(DMA2_Stream2, DMA_FLAG_TCIF2);

		//获得接收帧帧长
		USART1_Fram.AccessLen = TCP_MAX_LEN
				- DMA_GetCurrDataCounter(DMA2_Stream2);
		USART1_Fram.RxBuf[USART1_Fram.AccessLen] = '\0'; //添加结束符
		USART1_Fram.FinishFlag = 1;

		if (strchr((const char *) USART1_Fram.RxBuf, '$'))
		{
			localUpdate = true;
			USART1_Fram.FinishFlag = 0;
		}
		//这里可以通知任务来处理数据
#if SYSTEM_SUPPORT_OS
		//推送接收完成
		OSFlagPost((OS_FLAG_GRP*) &EventFlags,//对应的事件标志组
				(OS_FLAGS) FLAG_USART1_RxED,//事件位
				(OS_OPT) OS_OPT_POST_FLAG_SET,//选择置位
				(OS_ERR*) &err);//错误码
#endif
		//设置传输数据长度
		DMA_SetCurrDataCounter(DMA2_Stream2, TCP_MAX_LEN);
		//打开DMA
		DMA_Cmd(DMA2_Stream2, ENABLE);
	}
#if SYSTEM_SUPPORT_OS
	OSIntExit();    	//退出中断
#endif
}
