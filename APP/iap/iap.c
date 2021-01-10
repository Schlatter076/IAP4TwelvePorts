#include "iap.h"
#include "stmflash.h"
#include "usart.h"
#include "delay.h"
#include "F4G.h"
#include "wifi.h"
#include "md5.h"
#include "HTim.h"

pFunction Jump_To_Application;
uint32_t JumpAddress;

unsigned char flashBuf[128] =
{ 0 };
u16 flashCnt = 0;
u32 flashDes = ApplicationAddress;
uint32_t RamSource;
u16 flashSize = 0;

vu32 flagCPY = 0;

/************************************************************************/
void IAP_Init(void)
{
	if (MyFlashParams.DeviceID[0] != 'P')
	{
		flagCPY = MyFlashParams.IAPFlag;
		MyFlashParams.IAPFlag = IAP_SET_DeviceID_FLAG_DATA;
	}
}
/************************************************************************/
void IAP_RunApp(void)
{
	if (((*(__IO uint32_t*) ApplicationAddress) & 0x2FFE0000) == 0x20000000)
	{
		printf("\r\nRun to app.\r\n");
		RCC_DeInit(); //关闭时钟
		__disable_irq(); //关闭总中断
		//关闭外设
		TIM_Cmd(TIM3, DISABLE);
		USART_Cmd(USART1, DISABLE);
		USART_Cmd(USART2, DISABLE);
		USART_Cmd(USART3, DISABLE);

		JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		__set_MSP(*(__IO uint32_t*) ApplicationAddress);
		Jump_To_Application();
	}
}
/************************************************************************/
void IAP_Main_Menu(struct STRUCT_USART_Fram *fram)
{
	printf("Now enter the main menu!\r\n");

	while (1)
	{

	}
}
/************************************************************************/
int8_t IAP_Update(struct STRUCT_USART_Fram *fram)
{
	return 0;
}

/************************************************************************/
int8_t IAP_Upload(void)
{
	return 0;
}
/************************************************************************/
int8_t IAP_Erase(void)
{
	return 0;
}
u8 Set_DeviceID(void)
{
	return 0;
}
void Get_DeviceID(void)
{
}
