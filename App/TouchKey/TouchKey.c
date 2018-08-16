/*********************************************************************************************************
* 模块名称: TouchKey.c
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
#include "TouchKey.h"
#include "Lamp.h"
#include <stm32f10x_conf.h>

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
//static u8 touchFlag = 0;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigTouchKey(void);

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: ConfigTouchKey
* 函数功能: 配置TouchKey
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 20XX年XX月XX日
* 注    意: 
*********************************************************************************************************/
static void ConfigTouchKey(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;            //定义结构体GPIO_InitStructure,用来配置TouchKey的GPIO
  NVIC_InitTypeDef NVIC_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line5;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
//  
//  RCC_APB2PeriphClockCmd(USER_DEFINE_TOUCH_KEY_GPIO_CLK, ENABLE);   //使能TouchKey的时钟
//  
//  GPIO_InitStructure.GPIO_Pin = USER_DEFINE_TOUCH_KEY_GPIO_PIN;     //设置TouchKey的引脚
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;             //浮空输入模式
//  
//  GPIO_Init(USER_DEFINE_TOUCH_KEY_GPIO_PORT, &GPIO_InitStructure);  //根据参数初始化TouchKey的GPIO端口
}

/*********************************************************************************************************
* 函数名称: ConfigTouchKey
* 函数功能: 配置TouchKey
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 20XX年XX月XX日
* 注    意: 
*********************************************************************************************************/
void EXTI9_5_IRQHandler(void)
{
  static u8 status = 0;
  
  if(EXTI_GetITStatus(EXTI_Line5) != RESET)
  {
    status++;
    
    if(status >= 10)
    {
      status = 0;
    }
    
    SetLampBrightness(status);
    
    EXTI_ClearITPendingBit(EXTI_Line5);
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitTouchKey
* 函数功能: 初始化TouchKey
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 20XX年XX月XX日
* 注    意: 
*********************************************************************************************************/
void  InitTouchKey(void)
{
  ConfigTouchKey();   //配置TouchKey
}

///*********************************************************************************************************
//* 函数名称: InitTouchKey
//* 函数功能: 初始化TouchKey
//* 输入参数: void 
//* 输出参数: void
//* 返 回 值: void
//* 创建日期: 20XX年XX月XX日
//* 注    意: 
//*********************************************************************************************************/
//u8 ScanTouchKey(void)
//{ 
//  if(GPIO_ReadInputDataBit(USER_DEFINE_TOUCH_KEY_GPIO_PORT, 
//                                USER_DEFINE_TOUCH_KEY_GPIO_PIN) && touchFlag == 0)
//  {
//    touchFlag = 1;
//    
//    return 1;       
//  }
//  else
//  {
//    if(!GPIO_ReadInputDataBit(USER_DEFINE_TOUCH_KEY_GPIO_PORT, USER_DEFINE_TOUCH_KEY_GPIO_PIN))
//    {
//      touchFlag = 0;
//    }
//    
//    return 0;      
//  }
//}
