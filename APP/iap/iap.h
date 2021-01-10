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
void IAP_RunApp(void);
void IAP_Main_Menu(struct STRUCT_USART_Fram *fram);
int8_t IAP_Update(struct STRUCT_USART_Fram *fram);
int8_t IAP_Upload(void);
int8_t IAP_Erase(void);
u8 Set_DeviceID(void);
void Get_DeviceID(void);
#endif
