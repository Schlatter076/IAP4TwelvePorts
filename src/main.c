/* Includes */
#include "tcp_public.h"
#include "L74HC595.h"
#include "HTim.h"
#include "backlight.h"
#include "iap.h"

/**
 **===========================================================================
 **
 **  Abstract: main program
 **
 **===========================================================================
 */
int main(void)
{
	struct STRUCT_USART_Fram *fram = &USART1_Fram;
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
	delay_ms(1000);
	printf("{localUpdate?}\r\n");
	delay_ms(1000);
	delay_ms(1000);

	if (!localUpdate)
	{
		//需要连接wifi
		if (WIFI_Fram.AT_test_OK != 0 && MyFlashParams.WifiFlag == WIFI_FLAG)
		{
			WIFI_Fram.allowHeart = ConnectToServerByWIFI(TCP_IP, TCP_PORT);
		}
		if (WIFI_Fram.allowHeart == 0)
		{
			F4G_Fram.allowHeart = ConnectToServerBy4G(TCP_IP, TCP_PORT);
			fram = &F4G_Fram;
		}
		else
		{
			fram = &WIFI_Fram;
		}
	}

	while (1)
	{
		switch (MyFlashParams.IAPFlag)
		{
		case IAP_APPRUN_FLAG_DATA:  //jump to app
			IAP_RunApp();
			break;
		case IAP_INIT_FLAG_DATA:  //initialze state (blank mcu)
			IAP_Main_Menu(fram);
			break;
		case IAP_UPDATE_FLAG_DATA:  // download app state
			if (IAP_Update(fram))
			{
				NVIC_SystemReset();
			}
			else
			{
				MyFlashParams.IAPFlag = IAP_INIT_FLAG_DATA;
			}
			break;
		case IAP_UPLOAD_FLAG_DATA:  // upload app state
			IAP_Upload();
			MyFlashParams.IAPFlag = IAP_INIT_FLAG_DATA;
			break;
		case IAP_ERASE_FLAG_DATA:		// erase app state
			IAP_Erase();
			MyFlashParams.IAPFlag = IAP_INIT_FLAG_DATA;
			break;
		case IAP_SET_DeviceID_FLAG_DATA:		// set DeviceID
			Set_DeviceID();
			break;
		case IAP_GET_DeviceID_FLAG_DATA:		// read DeviceID
			Get_DeviceID();
			MyFlashParams.IAPFlag = IAP_INIT_FLAG_DATA;
			break;
		default:
			break;
		}
	}
}
