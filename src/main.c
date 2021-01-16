/* Includes */
#include "tcp_public.h"
#include "L74HC595.h"
#include "HTim.h"
#include "backlight.h"
#include "iap.h"
#include "usart.h"
#include "F4G.h"
#include "wifi.h"

/**
 **===========================================================================
 **
 **  Abstract: main program
 **
 **===========================================================================
 */
int main(void)
{
	u8 net = InUsart1;
	delay_init(168);  	//时钟初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  	//中断分组配置
	my_mem_init();
	STMFlash_Init();
	USART1_Init(115200);
	L74HC595_Init();
	Backlight_Init();
	TIM3_Init(1000 - 1, 8400 - 1); //定时器时钟84M 分频8400 周期=1000/10000S
	F4G_Init(115200);
	WIFI_Init(115200);
	IAP_Init(); //初始化
	//检查是否需要本地更新
	delay_ms(1000);
	printf("{localUpdate?}\r\n");
	delay_ms(1000);
	delay_ms(1000);
	if (!localUpdate)
	{
		//如果wifi上电成功
		if (Wifi_PowerOn())
		{
			net = InWifi;
		}
		else
		{
			F4G_PowerOn();
			net = In4G;
		}
	}
	while (1)
	{
		switch (MyFlashParams.IAPFlag)
		{
		case IAP_APPRUN_FLAG_DATA:  //jump to app
			if (IAP_RunApp())
			{
				MyFlashParams.IAPFlag = IAP_INIT_FLAG_DATA;
			}
			break;
		case IAP_INIT_FLAG_DATA:  //initialze state (blank mcu)
			IAP_Main_Menu(net);
			break;
		case IAP_UPDATE_FLAG_DATA:  // download app state
			if (IAP_Update(net))
			{
				NVIC_SystemReset();
			}
			else
			{
				MyFlashParams.IAPFlag = IAP_INIT_FLAG_DATA;
			}
			break;
		case IAP_SET_DeviceID_FLAG_DATA:		// set DeviceID
			IAP_Set_DeviceID(net);
			break;
		default:
			break;
		}
	}
}
