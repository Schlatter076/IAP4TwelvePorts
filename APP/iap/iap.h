#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"
#include "STMFlash.h"
#include "malloc.h"
#include "tcp_public.h"

/* Exported types ------------------------------------------------------------*/
typedef  void (*pFunction)(void);
extern pFunction Jump_To_Application;
extern uint32_t JumpAddress;

#define TCP_IP    "server.dayitc.com"
#define TCP_PORT  "5599"

#define ApplicationAddress    0x08010000

void IAP_Init(void);
u8 IAP_RunApp(void);
void IAP_Main_Menu(u8 Internet);
int8_t IAP_Update(u8 Internet);
u8 IAP_Set_DeviceID(u8 Internet);
#endif
