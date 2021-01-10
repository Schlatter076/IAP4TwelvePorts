/*
 * dma.h
 *
 *  Created on: 2021年1月6日
 *      Author: HHS007
 */

#ifndef DMA_DMA_H_
#define DMA_DMA_H_

#include "sys.h"

void DMA_USART_Tx_Init(USART_TypeDef* USARTx, uint32_t AHB1Periph,
		uint8_t IRQChannel, uint8_t IRQ_Pprio, uint8_t IRQ_Sprio,
		DMA_Stream_TypeDef* Streamx, uint32_t DChannel,
		uint32_t PeripheralBaseAddr, uint32_t Memory0BaseAddr,
		uint32_t BufferSize, uint32_t DMA_Priority);

void DMA_USART_Rx_Init(USART_TypeDef* USARTx, uint32_t AHB1Periph,
		DMA_Stream_TypeDef* Streamx, uint32_t DChannel,
		uint32_t PeripheralBaseAddr, uint32_t Memory0BaseAddr,
		uint32_t BufferSize, uint32_t DMA_Priority);

#endif /* DMA_DMA_H_ */
