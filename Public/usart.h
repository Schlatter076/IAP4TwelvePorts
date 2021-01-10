/*
 * usart.h
 *
 *  Created on: 2021年1月5日
 *      Author: HHS007
 */

#ifndef USART_H_
#define USART_H_

#include "sys.h"
#include "tcp_public.h"
#if SYSTEM_SUPPORT_OS
#include "includes.h"
#endif
#include "STMFlash.h"
#include "dma.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

extern bool localUpdate;

void USART1_Init(u32 bound);

#endif /* USART_H_ */
