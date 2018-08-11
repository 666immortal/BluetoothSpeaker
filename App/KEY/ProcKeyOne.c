/*********************************************************************************************************
* 模块名称: ProcKeyOne.c
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
#include "ProcKeyOne.h"
#include <stm32f10x_conf.h>
#include "UART.h"
#include "OLED.h"
#include "MusicPlayer.h"
#include "UI.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
extern u8 musicKey;
extern u8 NowInterFace;
extern u8 NowPointAt;

extern u8 oneSet;

extern u8 SetTimePressUp;  
extern u8 SetTimePressDown;
extern u8 SetTimePressSet;

extern u8 SetDatePressUp;
extern u8 SetDatePressDown;
extern u8 SetDatePressSet;

extern u8 SetAlarmPressUp;
extern u8 SetAlarmPressDown;
extern u8 SetAlarmPressSet;

extern u8 AlarmStop;

u8  Pressing   = 0;
u32 pressTime  = 0;

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
* 函数名称: InitProcKeyOne
* 函数功能: 初始化ProcKeyOne
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/
void InitProcKeyOne(void)
{

}

/*********************************************************************************************************
* 函数名称: ProcKeyDownKey0
* 函数功能: 处理按键0按下的事件，即按键按下的响应函数 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  ProcKeyDownKey0(void)       
{
  Pressing = 1;                       //按下
  
  printf("KEY0 PUSH DOWN\r\n");       //打印按键状态    
}

/*********************************************************************************************************
* 函数名称: ProcKeyUpKey0
* 函数功能: 处理按键0弹起的事件，即按键弹起的响应函数
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  ProcKeyUpKey0(void)
{
  if(Pressing)                              
  {
    if(pressTime >1200)                     
    {
      if(NowInterFace == MENU)            
      {
        
      }
      else if(NowInterFace == MUSIC)      
      {
        musicKey = NEXT_SONG;           
      }
      else if(NowInterFace == FM)         
      {
        
      }
      else if(NowInterFace == BLUETOOTH)
      {
        
      }      
    }
    else
    {
      if(NowInterFace == MENU)
      {
        if(NowPointAt >= PICK_BLUETOOTH)
        {
          NowPointAt = PICK_DATA;
        }
        else
        {
          NowPointAt++;
        }
      }
      else if(NowInterFace == MUSIC)
      {
        musicKey = VOL_UP;
      }
      else if(NowInterFace == FM)
      {
        
      }
      else if(NowInterFace == BLUETOOTH)    //处在蓝牙界面
      {
        
      }
      else if(NowInterFace == SET_TIME)
      {
        oneSet = 1;
        SetTimePressUp = 1;
      }
      else if (NowInterFace == SET_DATE)
      {
        oneSet = 1;
        SetDatePressUp = 1;
      }
      else if(NowInterFace == SET_ALARM)
      {
        oneSet = 1;
        SetAlarmPressUp = 1;
      }
      else if(NowInterFace == ALARM_RING)
      {
        
      }
    }
    
    Pressing  = 0;
    pressTime = 0;
  }
  
  printf("KEY0 PUSH UP\r\n");         //打印按键状态
}

/*********************************************************************************************************
* 函数名称: ProcKeyDownKey1
* 函数功能: 处理按键1按下的事件，即按键按下的响应函数 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  ProcKeyDownKey1(void)
{
  Pressing = 1;
  
  printf("KEY1 PUSH DOWN\r\n");      //打印按键状态
}

/*********************************************************************************************************
* 函数名称: ProcKeyUpKey1
* 函数功能: 处理按键1弹起的事件，即按键弹起的响应函数 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  ProcKeyUpKey1(void)
{
  if(Pressing)
  {
    if(pressTime >1200)
    {
      if(NowInterFace == MENU)
      {
        
      }
      else if(NowInterFace == MUSIC)
      {
        musicKey = QUIT;
      }
      else if(NowInterFace == FM)
      {
        
      }
      else if(NowInterFace == BLUETOOTH)
      {
        
      }     
    }
    else
    {    
      if(NowInterFace == MENU)
      {
        NowInterFace = NowPointAt;
      }
      else if(NowInterFace == MUSIC)
      {
        if(musicKey == PAUSE)
        {
          musicKey = 0;
        }
        else
        {
          musicKey = PAUSE;
        }
      }
      else if(NowInterFace == FM)
      {
        
      }
      else if(NowInterFace == BLUETOOTH)    //处在蓝牙界面
      {
        
      }  
      else if(NowInterFace == SET_TIME)     //处在设置时间界面
      {
        oneSet = 1;
        SetTimePressSet++;
      }
      else if (NowInterFace == SET_DATE)    //处在设置日期界面
      {
        oneSet = 1;
        SetDatePressSet++;
      } 
      else if(NowInterFace == SET_ALARM)    //处在设置闹钟界面
      {
        oneSet = 1;
        SetAlarmPressSet++;
      }
//      else if(NowInterFace == ALARM_RING)
//      {
//        AlarmStop = 0;
//      }
    }
    
    Pressing  = 0;
    pressTime = 0;
  }
  
  printf("KEY1 PUSH UP\r\n");         //打印按键状态
}

/*********************************************************************************************************
* 函数名称: ProcKeyDownKey2
* 函数功能: 处理按键2按下的事件，即按键按下的响应函数 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  ProcKeyDownKey2(void)
{
  Pressing = 1;
  
  printf("KEY2 PUSH DOWN\r\n");       //打印按键状态
}

/*********************************************************************************************************
* 函数名称: ProcKeyUpKey2
* 函数功能: 处理按键2弹起的事件，即按键弹起的响应函数 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  ProcKeyUpKey2(void)
{
  if(Pressing)
  {
    if(pressTime >1200)
    {
      if(NowInterFace == MENU)            //处在菜单界面
      {
        
      }
      else if(NowInterFace == MUSIC)      //处在播放器界面
      {
        musicKey = PREV_SONG;
      }
      else if(NowInterFace == FM)         //处在FM界面
      {
        
      }
      else if(NowInterFace == BLUETOOTH)
      {
        
      }      
    }
    else
    {
      if(NowInterFace == MENU)              //处在菜单界面
      {
        if(NowPointAt <= PICK_DATA)         //当光标处在日期处
        {
          NowPointAt = PICK_BLUETOOTH;      //将光标移到蓝牙
        }
        else
        {
          NowPointAt--;                     //光标左移一位
        }
      }
      else if(NowInterFace == MUSIC)        //处在播放器界面
      {
        musicKey = VOL_DOWN;                //音量减小
      }
      else if(NowInterFace == FM)           //处在FM界面
      {
        
      }
      else if(NowInterFace == BLUETOOTH)    //处在蓝牙界面
      {
        
      }   
      else if(NowInterFace == SET_TIME)     //处在设置时间界面
      {
        oneSet = 1;                         //
        SetTimePressDown = 1;               //
      }
      else if(NowInterFace == SET_DATE)     //处在设置日期界面
      {
        oneSet = 1;
        SetDatePressDown = 1;
      }
      else if(NowInterFace == SET_ALARM)    //处在设置闹钟界面
      {
        oneSet = 1;                         //
        SetAlarmPressDown = 1;              //
      }
      else if(NowInterFace == ALARM_RING)   //处于闹钟响铃界面
      {
        
      }
    }
    
    Pressing  = 0;                    //按下标志置0
    pressTime = 0;                    //按下的次数置0
  }
  
  printf("KEY2 PUSH UP\r\n");         //打印按键状态
}

/*********************************************************************************************************
* 函数名称: ProcKeyDownKeyUp
* 函数功能: 处理按键KEY_UP按下的事件，即按键按下的响应函数 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  ProcKeyDownKeyUp(void)
{
  
  printf("KEY_UP PUSH DOWN\r\n");      //打印按键状态
}

/*********************************************************************************************************
* 函数名称: ProcKeyUpKeyUp
* 函数功能: 处理按键KEY_UP弹起的事件，即按键弹起的响应函数 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  ProcKeyUpKeyUp(void)
{
  printf("KEY_UP PUSH UP\r\n");         //打印按键状态
}
