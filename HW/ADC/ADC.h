﻿/*********************************************************************************************************
* 模块名称: ADC.h
* 摘    要: ADC驱动
* 当前版本: 1.0.0
* 作    者: 
* 完成日期: 2018年03月01日
* 内    容:
* 注    意: none                                                                  
**********************************************************************************************************
* 取代版本:	 
* 作    者:
* 完成日期: 
* 修改内容:
* 修改文件: 
*********************************************************************************************************/
#ifndef _ADC_H_
#define _ADC_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "DataType.h"
#include <stm32f10x_conf.h>

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define USER_DEFINE_ADC                  ADC1                       //用户定义ADC
#define USER_DEFINE_ADC_CLK              RCC_APB2Periph_ADC1        //用户定义ADC的时钟
#define USER_DEFINE_ADC_CLK_DIVE         RCC_PCLK2_Div6             //用户定义ADC时钟的分频因子

#define USER_DEFINE_ADC_CHANNEL          ADC_Channel_1              //用户定义ADC的通道
#define USER_DEFINE_ADC_GPIO             GPIOA                      //用户定义ADC通道的GPIO端口
#define USER_DEFINE_ADC_GPIO_PIN         GPIO_Pin_1                 //用户定义ADC通道的GPIO的引脚
#define USER_DEFINE_ADC_GPIO_CLK         RCC_APB2Periph_GPIOA       //用户定义ADC通道的GPIO的时钟

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数定义
*********************************************************************************************************/
void InitADC(void);                    //初始化ADC
u16  GetADCAverage(u8 sampletimes);    //获取ADC采样数据的平均值
 
#endif 
