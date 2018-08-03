﻿/*********************************************************************************************************
* 模块名称: PWM.h
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
#ifndef _PWM_H_
#define _PWM_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "DataType.h"
#include <stm32f10x_conf.h>

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/
void  InitPWM(u16 arr, u16 psc);       //初始化

#endif
