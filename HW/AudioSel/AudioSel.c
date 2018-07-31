﻿/*********************************************************************************************************
* 模块名称: AudioSel.c
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

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "AudioSel.h"
#include "stm32f10x_gpio.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitAudioSel
* 函数功能: 初始化AudioSel
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  InitAudioSel(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;           //定义结构体GPIO_InitStructure,用来配置AudioSel的GPIO
  
  RCC_APB2PeriphClockCmd(USER_DEFINE_AUDIOSEL_A_CLK, ENABLE);         //使能Audiosel_A的时钟
  RCC_APB2PeriphClockCmd(USER_DEFINE_AUDIOSEL_B_CLK, ENABLE);         //使能Audiosel_B的时钟
  
  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_AUDIOSEL_A_PIN;         //设置Audiosel_A的引脚
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                   //设置I/O速率为50MHz
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;                   //推挽输出模式
  GPIO_Init(USER_DEFINE_AUDIOSEL_A_PORT, &GPIO_InitStructure);        //根据参数初始化Audiosel_A的GPIO端口
  
  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_AUDIOSEL_B_PIN;         //设置Audiosel_B的引脚
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                   //设置I/O速率为50MHz
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;                   //推挽输出模式
  GPIO_Init(USER_DEFINE_AUDIOSEL_B_PORT, &GPIO_InitStructure);        //根据参数初始化Audiosel_B的GPIO端口
  
  GPIO_WriteBit(USER_DEFINE_AUDIOSEL_A_PORT, USER_DEFINE_AUDIOSEL_A_PIN, Bit_RESET); //Audiosel_A默认值为0
  GPIO_WriteBit(USER_DEFINE_AUDIOSEL_B_PORT, USER_DEFINE_AUDIOSEL_B_PIN, Bit_RESET); //Audiosel_B默认值为0
}

/*********************************************************************************************************
* 函数名称: SetAudioSel
* 函数功能: 设置音频通道
* 输入参数: ch:选择的音频通道 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  SetAudioSel(u8 ch)
{
  GPIO_WriteBit(USER_DEFINE_AUDIOSEL_A_PORT, USER_DEFINE_AUDIOSEL_A_PIN, (BitAction)(ch & 0X01));
  GPIO_WriteBit(USER_DEFINE_AUDIOSEL_B_PORT, USER_DEFINE_AUDIOSEL_B_PIN, (BitAction)((ch >> 1) & 0X01));
}
