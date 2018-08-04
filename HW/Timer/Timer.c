﻿/*********************************************************************************************************
* 模块名称: Timer.c
* 摘    要: 
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

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "Timer.h"
#include "stm32f10x_tim.h"
#include "UART.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static  u8  s_i2msFlag  = FALSE;                 //2ms标志设置为 FALSE
static  u8  s_i1secFlag = FALSE;                 //100ms标志设置为 FALSE
static  u8  s_i100msFlag = FALSE;                //1s 标志设置为 FALSE
static  i32 s_iDbgAsstCnt = 0;                   //DbgAsst计时器

/*********************************************************************************************************
*                                              全局变量
*********************************************************************************************************/
extern u8  Pressing;
extern u32 pressTime;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static  void  ConfigTimer2(u16 arr, u16 psc);   //配置定时器TIM2的参数
static  void  ConfigTimer4(u16 arr, u16 psc);   //配置定时器TIM4的参数
static  void  ConfigTimer5(u16 arr, u16 psc);   //配置定时器TIM5的参数

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: ConfigTimer2
* 函数功能: 配置定时器TIM2的参数 
* 输入参数: arr：自动重装值；psc：时钟预分频数
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 通用定时器2中断初始化
*           这里时钟选择为APB1的2倍，而APB1为36M
*           arr：自动重装值；psc：时钟预分频数
*********************************************************************************************************/
static  void ConfigTimer2(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;//定义结构体TIM_TimeBaseStructure，用来配置定时器TIM2的参数
  NVIC_InitTypeDef NVIC_InitStructure;           //定义结构体NVIC_InitStructure，用来配置中断NVIC的参数

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  //使能定时器TIM2的时钟
  
  //定时器TIM2初始化
  TIM_TimeBaseStructure.TIM_Period        = arr;  //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
  TIM_TimeBaseStructure.TIM_Prescaler     = psc;  //设置用来作为TIMx时钟频率除数的预分频值
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;       //设置时钟分割:TDTS = Tck_tim
  TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up; //设置定时器TIM2为向上计数模式
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);               //根据指定的参数初始化TIM2的时间基数单位
                                                                
  TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE );                     //使能指定的定时器TIM2中断，并且允许更新中断
                                                                
  //中断优先级NVIC设置                                           
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;               //设置定时器TIM2的中断通道
  //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   //设置先占优先级为0级
  //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;          //设置从优先级为3级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;               //使能中断NVIC的中断通道
  NVIC_Init(&NVIC_InitStructure);                               //根据参数初始化中断NVIC的寄存器
                                                                
  TIM_Cmd(TIM2, ENABLE);                                        //使能定时器TIM2  
}

/*********************************************************************************************************
* 函数名称: ConfigTimer4
* 函数功能: 配置定时器TIM4的参数  
* 输入参数: arr：自动重装值；psc：时钟预分频数
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 通用定时器4中断初始化
*           这里时钟选择为APB1的2倍，而APB1为36M
*           arr：自动重装值；psc：时钟预分频数
*********************************************************************************************************/
static  void ConfigTimer4(u16 arr, u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;//定义结构体TIM_TimeBaseStructure，用来配置定时器TIM5的参数
  NVIC_InitTypeDef NVIC_InitStructure;           //定义结构体NVIC_InitStructure，用来配置中断NVIC的参数

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  //使能定时器TIM4的时钟
  
  //定时器TIM4初始化
  TIM_TimeBaseStructure.TIM_Period        = arr;  //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
  TIM_TimeBaseStructure.TIM_Prescaler     = psc;  //设置用来作为TIMx时钟频率除数的预分频值
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;       //设置时钟分割:TDTS = Tck_tim
  TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up; //设置定时器TIM4为向上计数模式
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);               //根据指定的参数初始化TIM4的时间基数单位
                                                                
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE );                   //使能指定的定时器TIM4中断,并且允许更新中断
                                                                
  //中断优先级NVIC设置                                           
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;               //设置定时器TIM4的中断通道
  //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   //设置先占优先级为0级
  //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;          //设置从优先级为3级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;               //使能中断NVIC的中断通道
  NVIC_Init(&NVIC_InitStructure);                               //根据参数初始化中断NVIC的寄存器
                                                                
  TIM_Cmd(TIM4, ENABLE);                                        //使能TIM4
}

/*********************************************************************************************************
* 函数名称: ConfigTimer5
* 函数功能: 配置定时器TIM5的参数  
* 输入参数: arr：自动重装值；psc：时钟预分频数
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 通用定时器5参数及其中断初始化
            这里时钟选择为APB1的2倍，而APB1为36M
*********************************************************************************************************/
static  void ConfigTimer5(u16 arr, u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure; //定义结构体TIM_TimeBaseStructure，用来配置定时器TIM5的参数
  NVIC_InitTypeDef NVIC_InitStructure;            //定义结构体NVIC_InitStructure，用来配置中断NVIC的参数

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);    //使能定时器TIM5的时钟
  
  //定时器TIM5初始化
  TIM_TimeBaseStructure.TIM_Period = arr;       //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
  TIM_TimeBaseStructure.TIM_Prescaler =psc;     //设置用来作为TIMx时钟频率除数的预分频值
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;       //设置时钟分割:TDTS = Tck_tim
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   //设置定时器TIM5为向上计数模式
  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);               //根据指定的参数初始化TIM5的时间基数单位
                                                                
  TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE );                     //使能指定的定时器TIM5中断,并且允许更新中断
                                                                
  //中断优先级NVIC设置                                           
  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;               //设置定时器TIM5的中断通道
  //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   //设置先占优先级为0级
  //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;          //设置从优先级为3级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;               //使能中断NVIC的中断通道
  NVIC_Init(&NVIC_InitStructure);                               //根据参数初始化中断NVIC的寄存器
                                                                
  TIM_Cmd(TIM5, ENABLE);                                        //使能定时器TIM5                                     
}

/*********************************************************************************************************
* 函数名称: TIM2_IRQHandler
* 函数功能: TIM2中断服务函数  
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 每隔1ms进入一次中断服务函数
*********************************************************************************************************/
void TIM2_IRQHandler(void)  
{
  static  i16 s_iCnt2   = 0;                          //定义一个静态变量s_iCnt2作为2ms计数器
  static  i16 s_iCnt100 = 0;                          //定义一个静态变量s_iCnt100作为100ms计数器
  
  if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)   //检查到TIM2更新中断的发生
  {                                                  
    TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);     //清除TIM2更新中断标志
  }                                                   
                                                      
  s_iCnt2++;                                          //2ms计数器的计数值加1
  if(s_iCnt2 >= 2)                                    //2ms计数器的计数值大于或等于2
  {                                                 
    s_iCnt2 = 0;                                      //重置2ms计数器的计数值为0
    s_i2msFlag = TRUE;                                //将2ms标志设置为TRUE
  }                                                  
                                                      
  s_iCnt100++;                                        //100ms计数器的计数值加1 
  if(s_iCnt100 >= 100)                                //100ms计数器的计数值大于或等于100
  {                                                   
    s_iCnt100 = 0;                                    //重置100ms计数器的计数值为0
    s_i100msFlag = TRUE;                              //将100ms标志设置为TRUE
  }                                                                                                        
}                                                     
                                                                                                          
/*********************************************************************************************************
* 函数名称: TIM4_IRQHandler                           
* 函数功能: TIM4中断服务函数  
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 每隔100ms进入一次中断服务函数
*********************************************************************************************************/
void TIM4_IRQHandler(void)  
{  
  if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)   //检查TIM4更新中断发生与否
  {
    TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update);     //清除TIM4中断标志
  }
}

/*********************************************************************************************************
* 函数名称: TIM5_IRQHandler
* 函数功能: TIM5中断服务函数  
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 每隔1ms进入一次中断服务函数
*********************************************************************************************************/
void TIM5_IRQHandler(void)  
{
  static  u16 s_iCnt1000  = 0;                       //定义一个静态变量s_iCnt1000作为1sec计数器
                                                     
  if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) //检查到TIM5更新中断的发生 
  {                                                  
    TIM_ClearITPendingBit(TIM5, TIM_FLAG_Update);    //清除TIM5更新中断标志  
  }   

  if(Pressing)
  {
    pressTime++;
  }
                                                     
  s_iCnt1000++;                                      //1000ms计数器的计数值大于或等于2
  if(s_iCnt1000 >= 1000)                             
  {                                                  //1000ms计数器的计数值超过1000
    s_iCnt1000 = 0;                                  
    s_i1secFlag = TRUE;                              //重置1000ms计数器的计数值为0
  }                                                  //将1s标志设置为TRUE
   
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitTimer
* 函数功能:  
* 输入参数: 
* 输出参数: 
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void InitTimer(void)
{
  ConfigTimer2(999, 71);      //1MHz，计数到999为1ms
  ConfigTimer4(9999, 719);    //72000000Hz/720=100KHz(0.01ms)，计数到9999为100ms
  ConfigTimer5(999, 71);      //1MHz，计数到999为1ms
}

/*********************************************************************************************************
* 函数名称: Get2msFlag
* 函数功能: 获取2ms标志  
* 输入参数: void
* 输出参数: s_i2msFlag
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8  Get2msFlag( void )
{
  return(s_i2msFlag);     //返回2ms标志
}

/*********************************************************************************************************
* 函数名称: Clr2msFlag
* 函数功能: 清除2ms标志  
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  Clr2msFlag(void)
{
  s_i2msFlag = FALSE;     //将2ms标志设置为FALSE 
}

/*********************************************************************************************************
* 函数名称: Get100msFlag
* 函数功能: 获取100ms标志  
* 输入参数: void
* 输出参数: s_i100msFlag
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8  Get100msFlag(void)
{
  return(s_i100msFlag);    //返回100ms标志
}

/*********************************************************************************************************
* 函数名称: Clr100msFlag
* 函数功能: 清除100ms标志  
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  Clr100msFlag(void)
{
  s_i100msFlag = FALSE;    //将100ms标志设置为FALSE
}

/*********************************************************************************************************
* 函数名称: Get1SecFlag
* 函数功能: 获取1sec标志  
* 输入参数: void
* 输出参数: s_i1secFlag
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8  Get1SecFlag(void)
{
  return(s_i1secFlag);    //返回1s标志
}

/*********************************************************************************************************
* 函数名称: Clr1SecFlag
* 函数功能: 清除1sec标志  
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  Clr1SecFlag(void)
{
  s_i1secFlag = FALSE;    //将1s标志设置为FALSE
}

/*********************************************************************************************************
* 函数名称: ResetDbgAsstTimer
* 函数功能: 复位调试助手定时器  
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void ResetDbgAsstTimer(void)
{
  TIM_ClearFlag(TIM4, TIM_FLAG_Update); //清除中断标志位 
  TIM_SetCounter(TIM4, 0);              //清空定时器的CNT
  s_iDbgAsstCnt = 0;                    //清空计数值
}

/*********************************************************************************************************
* 函数名称: GetDbgAsstTimer
* 函数功能: 获取调试助手定时器值
* 输入参数: void
* 输出参数: void
* 返 回 值: 时间值，单位为0.01ms，最大延时时间为定时器CNT*0.01ms
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
i32 GetDbgAsstTimer(void)
{  
  s_iDbgAsstCnt += TIM_GetCounter(TIM4);  //加上定时器TIM4的时间
  
  return s_iDbgAsstCnt;                   //返回计数值
}
