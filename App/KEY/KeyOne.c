/*********************************************************************************************************
* 模块名称: KeyOne.c
* 摘    要: 
* 当前版本: 1.0.0
* 作    者: 
* 完成日期: 2018年03月01日
* 内    容:
* 注    意: none                                                                  
**********************************************************************************************************
* 取代版本: 1.0.0
* 作    者:
* 完成日期: 
* 修改内容: 
* 修改文件: 
*********************************************************************************************************/

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "KeyOne.h"
#include <stm32f10x_conf.h>
#include "UART.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/  
//定义KEY0为在输入状态下I/O口的高低电平
#define  KEY0    GPIO_ReadInputDataBit(USER_DEFINE_KEY0_GPIO_PORT, USER_DEFINE_KEY0_GPIO_PIN) 
//定义KEY1为在输入状态下I/O口的高低电平
#define  KEY1    GPIO_ReadInputDataBit(USER_DEFINE_KEY1_GPIO_PORT, USER_DEFINE_KEY1_GPIO_PIN)
//定义KEY2为在输入状态下I/O口的高低电平
#define  KEY2    GPIO_ReadInputDataBit(USER_DEFINE_KEY2_GPIO_PORT, USER_DEFINE_KEY2_GPIO_PIN) 
//定义KEY_UP为在输入状态下I/O口的高低电平
#define  KEY_UP  GPIO_ReadInputDataBit(USER_DEFINE_KEY_UP_GPIO_PORT, USER_DEFINE_KEY_UP_GPIO_PIN)  
     
/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static  u8  s_arrKeyDownLevel[KEY_NAME_MAX];   //按键按下时的电压，0xFF表示按下为高电平，0x00表示按下为低电平

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static  void  ConfigKeyOneGPIO(void);          //配置按键的GPIO 

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: ConfigKeyOneGPIO
* 函数功能: 配置按键的GPIO 
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/
static  void  ConfigKeyOneGPIO(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;           //定义结构体GPIO_InitStructure，用来配置按键的GPIO
  
  RCC_APB2PeriphClockCmd(USER_DEFINE_KEY0_GPIO_CLK, ENABLE);     //使能KEY0的时钟
  RCC_APB2PeriphClockCmd(USER_DEFINE_KEY1_GPIO_CLK, ENABLE);     //使能KEY1的时钟
  RCC_APB2PeriphClockCmd(USER_DEFINE_KEY2_GPIO_CLK, ENABLE);     //使能KEY2的时钟
  RCC_APB2PeriphClockCmd(USER_DEFINE_KEY_UP_GPIO_CLK, ENABLE);   //使能KEY_UP的时钟
  
  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_KEY0_GPIO_PIN;     //设置KEY0的引脚
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;              //设置I/O口速率为10MHz
  GPIO_InitStructure.GPIO_Mode  = USER_DEFINE_KEY0_GPIO_MODE;    //设置KEY0的GPIO端口的模式
  GPIO_Init(USER_DEFINE_KEY0_GPIO_PORT, &GPIO_InitStructure);    //根据参数初始化KEY0的GPIO端口

  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_KEY1_GPIO_PIN;     //设置KEY1的引脚
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;              //设置I/O口速率为10MHz
  GPIO_InitStructure.GPIO_Mode  = USER_DEFINE_KEY1_GPIO_MODE;    //设置KEY1的GPIO端口的模式
  GPIO_Init(USER_DEFINE_KEY1_GPIO_PORT, &GPIO_InitStructure);    //根据参数初始化KEY1的GPIO端口

  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_KEY2_GPIO_PIN;     //设置KEY2的引脚
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;              //设置I/O口速率为10MHz
  GPIO_InitStructure.GPIO_Mode  = USER_DEFINE_KEY2_GPIO_MODE;    //设置KEY2的GPIO端口的模式
  GPIO_Init(USER_DEFINE_KEY2_GPIO_PORT, &GPIO_InitStructure);    //根据参数初始化KEY2的GPIO端口

  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_KEY_UP_GPIO_PIN;   //设置KEY_UP的引脚
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;              //设置I/O口速率为10MHz
  GPIO_InitStructure.GPIO_Mode  = USER_DEFINE_KEY_UP_GPIO_MODE;  //设置KEY_UP的GPIO端口的模式
  GPIO_Init(USER_DEFINE_KEY_UP_GPIO_PORT, &GPIO_InitStructure);  //根据参数初始化KEY_UP的GPIO端口
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitKeyOne
* 函数功能: 初始化KeyOne
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/
void InitKeyOne(void)
{
  ConfigKeyOneGPIO();                                          //配置按键的GPIO 

  s_arrKeyDownLevel[KEY_NAME_KEY0] = KEY_DOWN_LEVEL_KEY0;      //按键KEY0按下时为低电平
  s_arrKeyDownLevel[KEY_NAME_KEY1] = KEY_DOWN_LEVEL_KEY1;      //按键KEY1按下时为低电平
  s_arrKeyDownLevel[KEY_NAME_KEY2] = KEY_DOWN_LEVEL_KEY2;      //按键KEY2按下时为低电平
  s_arrKeyDownLevel[KEY_NAME_KEY_UP] = KEY_DOWN_LEVEL_KEY_UP;  //按键KEY_UP按下时为高电平

}

/***********************************************************************************************************
* 函数名称: ScanKeyOne
* 函数功能: 按键扫描，10ms扫描一次
* 输入参数: keyName-从头文件的枚举获取，OnKeyOneUp-按键弹起的响应函数的指针，OnKeyOneDown-按键按下的响应函数的指针
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 如果s_arrKeyDownLevel[keyName] = 0xFF，对s_arrKeyDownLevel[keyName]直接取反得出的是-256，而非0。
*           正确的做法是(u8)(~s_arrKeyDownLevel[keyName])，这样得出的才是0。
************************************************************************************************************/
void ScanKeyOne(u8 keyName, void(*OnKeyOneUp)(void), void(*OnKeyOneDown)(void))
{
  static  u8  s_arrKeyVal[KEY_NAME_MAX];         //定义一个u8类型的数组s_arrKeyVal[],存放按键的数值
  static  u8  s_arrKeyFlag[KEY_NAME_MAX];        //定义一个u8类型的数组s_arrKeyFlag[]，存放按键标志位
  
  s_arrKeyVal[keyName] = s_arrKeyVal[keyName] << 1;   //检查是否是有效操作，防抖动，80ms内的固定稳定操作才有效

  switch (keyName)
  {
    case KEY_NAME_KEY0:
      s_arrKeyVal[keyName] = s_arrKeyVal[keyName] | KEY0;   //有效按压/弹起时，按键0的低位到高位依次变为0/1（8位）
      break;
    case KEY_NAME_KEY1:                                 
      s_arrKeyVal[keyName] = s_arrKeyVal[keyName] | KEY1;   //有效按压/弹起时，按键1的低位到高位依次变为0/1（8位）
      break;
    case KEY_NAME_KEY2:
      s_arrKeyVal[keyName] = s_arrKeyVal[keyName] | KEY2;   //有效按压/弹起时，按键2的低位到高位依次变为0/1（8位）
      break;
    case KEY_NAME_KEY_UP:
      s_arrKeyVal[keyName] = s_arrKeyVal[keyName] | KEY_UP;//有效按压/弹起时，按键KEY_UP的低位到高位依次变为1/0（8位）
      break;
    default:
      break;
  }  
  
  //按键标志位为TRUE时，检测到有效的按压
  if(s_arrKeyVal[keyName] == s_arrKeyDownLevel[keyName] && s_arrKeyFlag[keyName] == TRUE)
  {
    (*OnKeyOneDown)();                      //处理按键按下的响应函数
    
    s_arrKeyFlag[keyName] = FALSE;          //表示按键处于按下状态，按键标志位变为FALSE，去除按键卡死影响。
  }
  
  //按键标志位为FALSE时，检测到有效的弹起
  else if(s_arrKeyVal[keyName] == (u8)(~s_arrKeyDownLevel[keyName]) && s_arrKeyFlag[keyName] == FALSE)
  {
    (*OnKeyOneUp)();                        //处理按键弹起的响应函数
    
    s_arrKeyFlag[keyName] = TRUE;           //表示按键处于弹起状态，按键标志位变为TRUE，去除按键停滞影响。
  }
}
