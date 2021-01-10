/*
 * dma.c
 *
 *  Created on: 2021年1月6日
 *      Author: HHS007
 */
#include "dma.h"

/**
 * DMA+串口发送配置
 * @USARTx  串口
 * @AHB1Periph DMA1 or DMA2
 * @IRQChannel DMA发送中断配置
 * @IRQ_Pprio  抢占优先级
 * @IRQ_Sprio  子优先级
 * @Streamx    DMA流
 * @DChannel   DMA通道
 * @PeripheralBaseAddr 外设基地址 如((uint32_t)&USART6->DR)
 * @Memory0BaseAddr    内存基地址,一般为发送缓冲区
 * @BufferSize   发送缓冲区的大小
 * @DMA_Priority DMA的优先级(非常高，高，低，非常低)
 */
void DMA_USART_Tx_Init(USART_TypeDef* USARTx, uint32_t AHB1Periph,
		uint8_t IRQChannel, uint8_t IRQ_Pprio, uint8_t IRQ_Sprio,
		DMA_Stream_TypeDef* Streamx, uint32_t DChannel,
		uint32_t PeripheralBaseAddr, uint32_t Memory0BaseAddr,
		uint32_t BufferSize, uint32_t DMA_Priority)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	//定义DMA结构体
	DMA_InitTypeDef DMA_InitStructure;
	//串口发DMA配置
	//启动DMA时钟
	RCC_AHB1PeriphClockCmd(AHB1Periph, ENABLE);
	//DMA发送中断设置
	NVIC_InitStructure.NVIC_IRQChannel = IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_Pprio;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = IRQ_Sprio;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//DMA通道配置
	DMA_DeInit(Streamx);

	DMA_InitStructure.DMA_Channel = DChannel;
	//外设地址
	DMA_InitStructure.DMA_PeripheralBaseAddr = PeripheralBaseAddr;
	//内存地址
	DMA_InitStructure.DMA_Memory0BaseAddr = Memory0BaseAddr;
	//dma传输方向
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	//设置DMA在传输时缓冲区的长度
	DMA_InitStructure.DMA_BufferSize = BufferSize;
	//设置DMA的外设递增模式，一个外设
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	//设置DMA的内存递增模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	//外设数据字长
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	//内存数据字长
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
	//设置DMA的传输模式
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	//设置DMA的优先级别
	DMA_InitStructure.DMA_Priority = DMA_Priority;

	//指定如果FIFO模式或直接模式将用于指定的流 ： 不使能FIFO模式
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	//指定了FIFO阈值水平
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	//指定的Burst转移配置内存传输
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	//指定的Burst转移配置外围转移 */
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	//配置DMA的通道
	DMA_Init(Streamx, &DMA_InitStructure);
	//使能中断
	DMA_ITConfig(Streamx, DMA_IT_TC, ENABLE);
	//采用DMA方式发送
	USART_DMACmd(USARTx, USART_DMAReq_Tx, ENABLE);
}
/**
 * DMA+串口接收配置
 * @USARTx  串口
 * @AHB1Periph DMA1 or DMA2
 * @Streamx    DMA流
 * @DChannel   DMA通道
 * @PeripheralBaseAddr 外设基地址 如((uint32_t)&USART6->DR)
 * @Memory0BaseAddr    内存基地址,一般为发送缓冲区
 * @BufferSize   发送缓冲区的大小
 * @DMA_Priority DMA的优先级(非常高，高，低，非常低)
 */
void DMA_USART_Rx_Init(USART_TypeDef* USARTx, uint32_t AHB1Periph,
		DMA_Stream_TypeDef* Streamx, uint32_t DChannel,
		uint32_t PeripheralBaseAddr, uint32_t Memory0BaseAddr,
		uint32_t BufferSize, uint32_t DMA_Priority)
{
	//定义DMA结构体
	DMA_InitTypeDef DMA_InitStructure;
	//串口收DMA配置
	//启动DMA时钟
	RCC_AHB1PeriphClockCmd(AHB1Periph, ENABLE);
	//DMA通道配置
	DMA_DeInit(Streamx);
	DMA_InitStructure.DMA_Channel = DChannel;
	//外设地址
	DMA_InitStructure.DMA_PeripheralBaseAddr = PeripheralBaseAddr;
	//内存地址
	DMA_InitStructure.DMA_Memory0BaseAddr = Memory0BaseAddr;
	//dma传输方向
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	//设置DMA在传输时缓冲区的长度
	DMA_InitStructure.DMA_BufferSize = BufferSize;
	//设置DMA的外设递增模式，一个外设
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	//设置DMA的内存递增模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	//外设数据字长
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	//内存数据字长
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	//设置DMA的传输模式
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	//设置DMA的优先级别
	DMA_InitStructure.DMA_Priority = DMA_Priority;

	//指定如果FIFO模式或直接模式将用于指定的流 ： 不使能FIFO模式
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	//指定了FIFO阈值水平
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	//指定的Burst转移配置内存传输
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	//指定的Burst转移配置外围转移 */
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	//配置DMA的通道
	DMA_Init(Streamx, &DMA_InitStructure);
	//使能通道
	DMA_Cmd(Streamx, ENABLE);
	//采用DMA方式接收
	USART_DMACmd(USARTx, USART_DMAReq_Rx, ENABLE);
}

