/*
 * wifi.h
 *
 *  Created on: 2021年1月8日
 *      Author: HHS007
 */

#ifndef WIFI_WIFI_H_
#define WIFI_WIFI_H_

#include "tcp_public.h"
#include "dma.h"
#include "iap.h"

#define _WIFI_RST   (PAout(1))

void WIFI_Init(u32 bound);
void WIFI_Send(const char *data);
u8 WIFI_Get_LinkStatus(void);
bool ConnectToServerByWIFI(char* addr, char* port);
u8 Wifi_PowerOn(void);

#endif /* WIFI_WIFI_H_ */
