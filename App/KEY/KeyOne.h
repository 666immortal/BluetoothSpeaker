/*********************************************************************************************************
* 模块名称: KeyOne.h
* 摘    要: KeyOne驱动
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
#ifndef _KEY_ONE_H_
#define _KEY_ONE_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "DataType.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
//各个按键按键按下的电平
#define  KEY_DOWN_LEVEL_KEY0             0x00                        //0x00表示按下为低电平
#define  KEY_DOWN_LEVEL_KEY1             0x00                        //0x00表示按下为低电平
#define  KEY_DOWN_LEVEL_KEY2             0x00                        //0x00表示按下为低电平
#define  KEY_DOWN_LEVEL_KEY_UP           0xFF                        //0xFF表示按下为高电平

#define USER_DEFINE_KEY0_GPIO_PORT       GPIOE                       //用户定义KEY0的GPIO端口
#define USER_DEFINE_KEY0_GPIO_PIN        GPIO_Pin_4                  //用户定义KEY0的引脚
#define USER_DEFINE_KEY0_GPIO_CLK        RCC_APB2Periph_GPIOE        //用户定义KEY0的时钟
#define USER_DEFINE_KEY0_GPIO_MODE       GPIO_Mode_IPU               //用户定义KEY0的GPIO端口的模式

#define USER_DEFINE_KEY1_GPIO_PORT       GPIOE                       //用户定义KEY1的GPIO端口
#define USER_DEFINE_KEY1_GPIO_PIN        GPIO_Pin_3                  //用户定义KEY1的引脚
#define USER_DEFINE_KEY1_GPIO_CLK        RCC_APB2Periph_GPIOE        //用户定义KEY1的时钟
#define USER_DEFINE_KEY1_GPIO_MODE       GPIO_Mode_IPU               //用户定义KEY1的GPIO端口的模式

#define USER_DEFINE_KEY2_GPIO_PORT       GPIOE                       //用户定义KEY2的GPIO端口
#define USER_DEFINE_KEY2_GPIO_PIN        GPIO_Pin_2                  //用户定义KEY2的引脚
#define USER_DEFINE_KEY2_GPIO_CLK        RCC_APB2Periph_GPIOE        //用户定义KEY2的时钟
#define USER_DEFINE_KEY2_GPIO_MODE       GPIO_Mode_IPU               //用户定义KEY2的GPIO端口的模式

#define USER_DEFINE_KEY_UP_GPIO_PORT     GPIOA                       //用户定义KEY_UP的GPIO端口
#define USER_DEFINE_KEY_UP_GPIO_PIN      GPIO_Pin_0                  //用户定义KEY_UP的引脚
#define USER_DEFINE_KEY_UP_GPIO_CLK      RCC_APB2Periph_GPIOA        //用户定义KEY_UP的时钟
#define USER_DEFINE_KEY_UP_GPIO_MODE     GPIO_Mode_IPD               //用户定义KEY_UP的GPIO端口的模式

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/
typedef enum
{
  KEY_NAME_KEY0 = 0,     //按键0
  KEY_NAME_KEY1,         //按键1
  KEY_NAME_KEY2,         //按键2
  KEY_NAME_KEY_UP,       //按键KEY_UP
  KEY_NAME_MAX
}EnumKeyOneName;

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/
void  InitKeyOne(void);                   //初始化按键
void  ScanKeyOne(u8 keyName, void(*OnKeyOneUp)(void), void(*OnKeyOneDown)(void));    //扫描按键

#endif
