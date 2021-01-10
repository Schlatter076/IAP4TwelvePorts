/*
 * HTim.h
 *
 *  Created on: 2021年1月7日
 *      Author: HHS007
 */

#ifndef HTIM_HTIM_H_
#define HTIM_HTIM_H_

#include "backlight.h"
#include "tcp_public.h"
#include <stdbool.h>
#if SYSTEM_SUPPORT_OS
#include "includes.h"
#endif

extern bool updating;

void TIM3_Init(u16 arr, u16 psc);

#endif /* HTIM_HTIM_H_ */
