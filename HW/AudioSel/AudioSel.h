﻿/*********************************************************************************************************
* 模块名称: AudioSel.h
* 摘    要: 
* 当前版本: 1.0.0
* 作    者: 
* 完成日期: 
* 内    容:
* 注    意: none                                                                  
**********************************************************************************************************
* 取代版本: 
* 作    者:
* 完成日期: 
* 修改内容:
* 修改文件: 
*********************************************************************************************************/
#ifndef _AUDIO_SEL_H_
#define _AUDIO_SEL_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "DataType.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define  USER_DEFINE_AUDIOSEL_A_PORT  GPIOC
#define  USER_DEFINE_AUDIOSEL_A_PIN   GPIO_Pin_6
#define  USER_DEFINE_AUDIOSEL_A_CLK   RCC_APB2Periph_GPIOC

#define  USER_DEFINE_AUDIOSEL_B_PORT  GPIOC
#define  USER_DEFINE_AUDIOSEL_B_PIN   GPIO_Pin_7
#define  USER_DEFINE_AUDIOSEL_B_CLK   RCC_APB2Periph_GPIOC

//声音通道选择
#define AUDIO_MP3     0   //MP3通道
#define AUDIO_RADIO   1   //收音机通道
#define AUDIO_BT      2   //蓝牙音频通道
#define AUDIO_NONE    3   //无声

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/
void  InitAudioSel(void);       //初始化音频通道
void  SetAudioSel(u8 ch);       //设置音频通道

#endif
