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
u32 flashDes = ApplicationAddress;
uint32_t RamSource;
u16 flashSize = 0;

/************************************************************************/
void IAP_Init(void)
{
	if (MyFlashParams.DeviceID[0] != 'P')
	{
		MyFlashParams.IAPFlag = IAP_SET_DeviceID_FLAG_DATA;
	}
}
/************************************************************************/
u8 IAP_RunApp(void)
{
	uint32_t data = (*(__IO uint32_t*) ApplicationAddress);
	if ((data & 0x2FFE0000) == 0x20020000)
	{
		DEBUG("\r\nRun to app.\r\n");
		//跳转之前写入flash参数
		MyFlash_Write(&MyFlashParams);
		RCC_DeInit(); //关闭时钟
		__disable_irq(); //关闭总中断
		//关闭外设
		TIM_Cmd(TIM3, DISABLE);
		DMA_Cmd(DMA2_Stream7, DISABLE);
		DMA_Cmd(DMA2_Stream2, DISABLE);
		USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
		USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
		USART_Cmd(USART1, DISABLE);
		DMA_Cmd(DMA1_Stream6, DISABLE);
		DMA_Cmd(DMA1_Stream5, DISABLE);
		USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
		USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
		USART_Cmd(USART2, DISABLE);
		DMA_Cmd(DMA1_Stream3, DISABLE);
		DMA_Cmd(DMA1_Stream1, DISABLE);
		USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
		USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
		USART_Cmd(USART3, DISABLE);

		JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		__set_MSP(*(__IO uint32_t*) ApplicationAddress);
		Jump_To_Application();
		return 0;
	}
	else
	{
		DEBUG("\r\nRun to app fail.\r\n");
		return 1;
	}
}
/************************************************************************/
void IAP_Main_Menu(u8 net)
{
	u8 iapREQ_cnt = 0;
	u8 cmd = 0;
	char *buf = mymalloc(BASE64_BUF_LEN);
	memset(buf, '\0', BASE64_BUF_LEN);
	DEBUG("Now enter the main menu!\r\n");
	while (1)
	{
		if (iapREQ_cnt++ >= 3)  //请求3次无响应，重启整个应用
		{
			NVIC_SystemReset();
		}
		IAP_request4Register(net);
		if (IAP_analyse(net, &cmd, buf, BASE64_BUF_LEN, 100))
		{
			IAP_process(net, cmd, buf);
			myfree(buf);
			break;
		}
	}
}
/************************************************************************/
int8_t IAP_Update(u8 net)
{
	char *result = NULL;
	char md5[33];
	memset(md5, '\0', 33);
	u32 appBytesSize = 0;
	u16 frameSize = 0;
	u16 frameCounter = 0;
	u16 step = 0;
	char m[33];
	char *BinStr = NULL;

	u8 cmd = 0;
	char *ServerData;
	ServerData = mymalloc(BASE64_BUF_LEN);
	memset(ServerData, '\0', BASE64_BUF_LEN);
	DEBUG("Update begin.\r\n");
	updating = true; //开始更新
	while (1)
	{
		IAP_request4Update(net);
		if (IAP_analyse(net, &cmd, ServerData, BASE64_BUF_LEN, 100))
		{
			if (cmd == DOWN_IAP_CurrentVersion)  //当前的版本信息
			{
				result = strtok(ServerData, "-");
				if (result != NULL)
				{
					strcpy((char *) MyFlashParams.Version, result);
					MyFlashParams.Version[20] = '\0';
				}
				result = strtok(NULL, "-");
				appBytesSize = result == NULL ? 0 : strtoul(result, NULL, 10);
				result = strtok(NULL, "-");
				frameSize = result == NULL ? 0 : strtoul(result, NULL, 10);
				result = strtok(NULL, "-");
				if (result != NULL)
				{
					memcpy(md5, result, 32);
				}
				//判定数据是否接收正确
				if (strlen((const char *) MyFlashParams.Version) > 2
						&& appBytesSize != 0 && frameSize != 0
						&& strlen(md5) == 32)
				{
					DEBUG("ver=%s,appSize=%ld,frameSize=%d,md5=%s\r\n",
							MyFlashParams.Version, appBytesSize, frameSize,
							md5);
					break;
				}
				else
				{
					goto ERROR;
				}
			}
			break;
		}
	}
	//获取帧数
	frameCounter =
			(appBytesSize % frameSize == 0) ?
					appBytesSize / frameSize : (appBytesSize / frameSize + 1);
	//擦除对应扇区(扇区4有128K大小，足够应用使用)
	FLASH_Unlock();	//解锁
	FLASH_DataCacheCmd(DISABLE); //FLASH擦除期间,必须禁止数据缓存
	FLASH_EraseSector(FLASH_Sector_4, VoltageRange_3);		//VCC=2.7~3.6V之间!!
	FLASH_DataCacheCmd(ENABLE);	//FLASH擦除结束,开启数据缓存
	FLASH_Lock();	//上锁
	memset(ServerData, '\0', BASE64_BUF_LEN);
	//开始请求更新
	for (int num = 0; num < frameCounter; num++)
	{
		do
		{
			IAP_request4_NFrame(net, num);
		} while (!IAP_analyse(net, &cmd, ServerData, BASE64_BUF_LEN, 100));
//		if (IAP_analyse(net, &cmd, ServerData, BASE64_BUF_LEN, 200))
//		{
		if (cmd == DOWN_IAP_N_Frame)
		{
			step = atoi(ServerData);
			if (step == num) //确定是否是当前帧
			{
				result = strchr(ServerData, '(');
				result += 1; //跳过字符'('
				//开始获取二进制文件
				BinStr = strtok(result, ","); //得到当前帧的第一个字节
				flashBuf[0] = atoi(BinStr);
				DEBUG("frame%d=(%02X,", num, flashBuf[0]);
				for (u16 flashCnt = 1; flashCnt < frameSize; flashCnt++)
				{
					BinStr = strtok(NULL, ",");
					flashBuf[flashCnt] = atoi((const char *) BinStr);
					DEBUG("%02X,", flashBuf[flashCnt]);
				}
				DEBUG(")\r\n");
				RamSource = (uint32_t) flashBuf;
				for (u32 j = 0;
						(j < frameSize)
								&& (flashDes < ApplicationAddress + appBytesSize);
						j += 4)
				{
					FLASH_Unlock();
					FLASH_ProgramWord(flashDes, *(uint32_t *) RamSource);
					FLASH_Lock();
					if (*(uint32_t *) flashDes != *(uint32_t *) RamSource)
					{
						DEBUG("write to flash error,addr=%08X\r\n", flashDes);
						goto ERROR;
					}
					flashDes += 4;
					RamSource += 4;
				}

			}
			else
			{
				goto ERROR;
			}
		}
		else
		{
			goto ERROR;
		}
//		}
	}
	//数据全部写入了flash
	//进行MD5校验
	memset(m, '\0', 33);
	getMD5Str(m, flashBuf, 128);
	if (strcmp(m, md5) == 0) //MD5校验成功
	{
		DEBUG("All datas has been recieved,md5=%s!\r\n", m);
		IAP_request4UpdateComp(net);
		if (IAP_analyse(net, &cmd, ServerData, BASE64_BUF_LEN, 100))
		{
			IAP_process(net, cmd, ServerData);
			myfree(ServerData);
			updating = false;
			return 1;
		}
	}
	//这里是出错后数据解析
	ERROR: IAP_request4Error(net); //出错了
	if (IAP_analyse(net, &cmd, ServerData, BASE64_BUF_LEN, 100))
	{
		IAP_process(net, cmd, ServerData);
	}
	myfree(ServerData);
	updating = false;
	return 0;
}
u8 IAP_Set_DeviceID(u8 net)
{
	char *temp = NULL;
	u16 timeout = 5;
	if (net == InUsart1)  //是要本地更新
	{
		do
		{
			DEBUG("{DeviceID}\r\n");
			while (timeout--)
			{
				delay_ms(100);
				if (USART1_Fram.FinishFlag)
				{
					USART1_Fram.FinishFlag = 0;
					temp = strchr((const char *) USART1_Fram.RxBuf, '[');
					if (temp && strchr((const char *) USART1_Fram.RxBuf, ']'))
					{
						temp += 1; //跳过字符'['
						temp = strtok(temp, "]"); //截取设备编号
						if (strlen(temp) == 8)  //设备编号长度为8
						{
							strcpy((char *) MyFlashParams.DeviceID, temp);
							MyFlashParams.DeviceID[8] = '\0';
							break;
						}
					}
				}
			}
			if (timeout) //已经成功设置了ID
			{
				break;
			}
			else
			{
				timeout = 100; //下一次开始
			}
		} while (1);
	}
	else
	{
		//设置默认编号
		MyFlashParams.DeviceID[0] = 'X';
		for (int i = 13; i < 20; i++)
		{
			MyFlashParams.DeviceID[i - 12] = MyFlashParams.ccid[i];
		}
		MyFlashParams.DeviceID[8] = '\0';
		DEBUG("Now set DeviceID=%s.\r\n", MyFlashParams.DeviceID);
	}
	MyFlashParams.IAPFlag = IAP_INIT_FLAG_DATA; //让设备进入初始化状态
	return 0;
}
