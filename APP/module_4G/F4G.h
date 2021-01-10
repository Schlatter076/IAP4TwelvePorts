/*
 * F4G.h
 *
 *  Created on: 2021年1月8日
 *      Author: HHS007
 */

#ifndef MODULE_4G_F4G_H_
#define MODULE_4G_F4G_H_

#include "sys.h"
#include "dma.h"
#include "tcp_public.h"
#include <stdbool.h>

#define _F4G_PKEY    (PEout(14))
#define _F4G_RST     (PEout(15))

void F4G_Init(u32 bound);
bool ConnectToServerBy4G(char* addr, char* port);
void getModuleMes(void);
void Module4G_Send(const char *data);

#endif /* MODULE_4G_F4G_H_ */
