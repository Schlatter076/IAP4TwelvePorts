/*
 * HTim.c
 *
 *  Created on: 2021年1月7日
 *      Author: HHS007
 */
#include "HTim.h"

bool updating = false;

u16 init_cnt = 0;
u16 update_cnt = 0;

/**
 * 通用定时器3初始化
 * @arr 自动重装值
 * @psc 预分频系数
 * 溢出时间Tout=((arr+1)*(psc+1))/Ft us
 * Ft=定时器工作频率，MHz
 */
void TIM3_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  ///使能定时器3时钟

	TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装值
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;  //分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure); //初始化TIM3

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); //允许更新中断
	TIM_Cmd(TIM3, ENABLE); //使能定时器

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; //中断通道配置
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM3_IRQHandler(void)
{
#if SYSTEM_SUPPORT_OS  //使用UCOS操作系统
	OSIntEnter();
#endif
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) //溢出中断
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除标志位

		if (init_cnt++ == 5)
		{
			init_cnt = 0;
			BACK_LIGHT_STA ^= 1;
		}
		if (updating)
		{
			if (update_cnt++ == 2)
			{
				update_cnt = 0;
				BACK_LIGHT_STA ^= 1;
			}
		}
	}
#if SYSTEM_SUPPORT_OS
	OSIntExit(); //退出中断
#endif
}

