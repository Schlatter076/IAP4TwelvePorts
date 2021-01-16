/*
 * STMFlash.h
 *
 *  Created on: 2021年1月5日
 *      Author: HHS007
 */

#ifndef FLASH_STMFLASH_H_
#define FLASH_STMFLASH_H_

#include "sys.h"
#include <string.h>

//FLASH起始地址
#define STM32_FLASH_BASE  ((u32)0x08000000) 	//STM32 FLASH的起始地址
//参数
#define DEVICE_ID_ADDR    ((u32)0x080E0000)   //设备编号-长度8字节
#define VERSION_ADDR      ((u32)0x080E0008)   //程序版本-长度20字节
#define APP_SERVER_ADDR   ((u32)0x080E001C)   //服务器拿回的数据-长度100字节
#define IGNORE_LOCK_ADDR  ((u32)0x080E0080)   //锁忽略状态-长度12字节
#define WIFI_FLAG_ADDR    ((u32)0x080E008C)   //是否设置wifi标志位-4字节
#define WIFI_SSID_ADDR    ((u32)0x080E0090)   //wifi名-长度100字节
#define WIFI_PWD_ADDR     ((u32)0x080E00F4)   //wifi密码-长度100字节
//IAP相关
#define IAP_INIT_FLAG_DATA          ((u32)0x0000FFFF)   //默认标志的数据(空片子的情况)
#define IAP_UPDATE_FLAG_DATA        ((u32)0x0000EEEE)   //下载标志的数据
#define IAP_UPLOAD_FLAG_DATA        ((u32)0x0000DDDD)   //上传标志的数据
#define IAP_ERASE_FLAG_DATA         ((u32)0x0000CCCC)   //擦除标志的数据
#define IAP_APPRUN_FLAG_DATA        ((u32)0x00005A5A)   //APP不需要做任何处理，直接运行状态
#define IAP_SET_DeviceID_FLAG_DATA  ((u32)0x0000EDED)  //写设备标号
#define IAP_GET_DeviceID_FLAG_DATA  ((u32)0x0000DEDE)  //读设备标号
//模块参数信息
#define CCID_ADDR     			((u32)0x080E0158)   //物联网卡号,长度20字节
#define COPS_ADDR     			((u32)0x080E016C)   //运营商信息-长度1字节

//FLASH 扇区的起始地址
#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	//扇区0起始地址, 16 Kbytes
#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	//扇区1起始地址, 16 Kbytes
#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	//扇区2起始地址, 16 Kbytes
#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	//扇区3起始地址, 16 Kbytes
#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	//扇区4起始地址, 64 Kbytes
#define ADDR_FLASH_SECTOR_5     ((u32)0x08020000) 	//扇区5起始地址, 128 Kbytes
#define ADDR_FLASH_SECTOR_6     ((u32)0x08040000) 	//扇区6起始地址, 128 Kbytes
#define ADDR_FLASH_SECTOR_7     ((u32)0x08060000) 	//扇区7起始地址, 128 Kbytes
#define ADDR_FLASH_SECTOR_8     ((u32)0x08080000) 	//扇区8起始地址, 128 Kbytes
#define ADDR_FLASH_SECTOR_9     ((u32)0x080A0000) 	//扇区9起始地址, 128 Kbytes
#define ADDR_FLASH_SECTOR_10    ((u32)0x080C0000) 	//扇区10起始地址,128 Kbytes
#define ADDR_FLASH_SECTOR_11    ((u32)0x080E0000) 	//扇区11起始地址,128 Kbytes

#define WIFI_FLAG  ((u32)0x5746)

struct AboutFlash_typeDef
{
	u8 DeviceID[9];
	u8 Version[21];
	u8 ServerParams[101];
	u8 IgnoreLock[12];
	vu32 WifiFlag;
	u8 WifiSSID[101];
	u8 WifiPWD[101];
	vu32 IAPFlag;
	u8 ccid[21];
	u8 cops;
	u8 rssi; //信号强度
};
extern struct AboutFlash_typeDef MyFlashParams;

//u32 STMFLASH_ReadWord(u32 faddr);		  	//读出字
//void STMFLASH_Write(u32 WriteAddr, u32 *pBuffer, u32 NumToWrite);//从指定地址开始写入指定长度的数据
//void STMFLASH_Read(u32 ReadAddr, u32 *pBuffer, u32 NumToRead); //从指定地址开始读出指定长度的数据

void STMFlash_Init(void);
void MyFlash_Write(struct AboutFlash_typeDef *def);

#endif /* FLASH_STMFLASH_H_ */
