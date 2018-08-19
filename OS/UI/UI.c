/*********************************************************************************************************
* 模块名称: UI.c
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
#include "UI.h"
#include "Text.h"
#include "OLED.h"
#include "vs10xx.h"
#include "MusicPlayer.h"
#include "SysTick.h"
#include "UART.h"
#include "RTC.h"
#include "KeyOne.h"
#include "ProcKeyOne.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
StructMaskIcon menuMask;

u8 oneSet            = 0;
                     
u8 SetTimePressUp    = 0;
u8 SetTimePressDown  = 0;
u8 SetTimePressSet   = 0;
                     
u8 SetDatePressUp    = 0;
u8 SetDatePressDown  = 0;
u8 SetDatePressSet   = 0;

u8 SetAlarmPressUp   = 0;
u8 SetAlarmPressDown = 0;
u8 SetAlarmPressSet  = 0;

u8 AlarmStop = 0;

extern u8 NowInterFace;   //引进外部变量NowInterFace
extern u8 NowPointAt;     //引进外部变量NowPointAt

extern u8 nowAlarmHour;
extern u8 nowAlarmMin;
extern u8 AlarmState;

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
* 函数名称: InitUI
* 函数功能: 初始化UI
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  InitUI(void)
{
  menuMask.maskAlarm = 1;
  menuMask.maskBT    = 1;
  menuMask.maskMP    = 0;
  menuMask.maskFM    = 1;
  menuMask.maskDate  = 1;
  menuMask.maskTime  = 1;
  menuMask.RTbettery = S_BATTERY_3;
  
  Show_Str(16, 0, 120, 24, "肖忆衷带李灰出品", 12, 1); 
  OLEDRefreshGRAM();
  DelayNms(2000); 
}

/*********************************************************************************************************
* 函数名称: ShowDesktop
* 函数功能: 初始化UI
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void ShowMenu(void)
{
  OLEDClear();
//  Show_Str(0, 2, 50, 16, "10月10日", 12, menuMask.maskDate);
//  OLEDShowString(53, 2, "16:66", 12, maskTime);
  OLEDShowNum(0, 2, calendar.month, 2, 12, menuMask.maskDate);
  OLEDShowNum(24,2, calendar.date, 2, 12, menuMask.maskDate);
  Show_Str(12, 2, 12, 12, "月", 12, menuMask.maskDate);
  Show_Str(36, 2, 12, 12, "日", 12, menuMask.maskDate);
  OLEDShowNum(53, 2, calendar.hour, 2, 12, menuMask.maskTime);
  OLEDShow0Num(71, 2, calendar.min, 2, 12, menuMask.maskTime);
  if(AlarmState == 1)
  {
    OLEDShowIcon(86, 0, S_ALARM, menuMask.maskAlarm);
  }
  else if(AlarmState == 0)
  {
    OLEDShowIcon(86, 0, S_ALARM_CLOSE, menuMask.maskAlarm);
  }
  OLEDShowIcon(108, 0, menuMask.RTbettery, 1);
  OLEDShowIcon(0, 20, B_FM, menuMask.maskFM);
  OLEDShowIcon(43, 20, B_MUSIC, menuMask.maskMP);
  OLEDShowIcon(86, 20, B_BLUETOOTH, menuMask.maskBT);
  OLEDShowChar(65, 2, ':', 12, menuMask.maskTime);
//  Show_Font(12, 2, "月", 12, menuMask.maskDate);
//  Show_Font(36, 2, "日", 12, menuMask.maskDate);
  
  Show_Str(3, 48, 40, 12, "收音机", 12, 1);
  Show_Str(46, 48, 40, 12, "播放器", 12 ,1);
  Show_Str(95, 48, 30, 12, "蓝牙", 12, 1);
  
  OLEDRefreshGRAM();
}

/*********************************************************************************************************
* 函数名称: MusicInterface
* 函数功能: 音乐播放器
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void OperateMusicPlayer(void)
{
  OLEDClear();
  Show_Str(0, 0, 128, 64, "存储器测试...", 12, 1);
  OLEDRefreshGRAM();
  OLEDClear();
	printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//打印RAM测试结果	    
	Show_Str(0, 0, 128, 64, "正弦波测试...", 12, 1); 	 
  OLEDRefreshGRAM();
 	VS_Sine_Test();	   
  OLEDClear();
	Show_Str(0, 0, 128, 64, "<<音乐播放器>>", 12, 1); 		 
  OLEDRefreshGRAM();
	mp3_play();
}

/*********************************************************************************************************
* 函数名称: SetTimeFace
* 函数功能: 设置时间界面
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void UpdateMask(u8 state)
{
  switch(state)
  {
    case PICK_DATA:
      menuMask.maskDate  = 0;
      menuMask.maskTime  = 1;
      menuMask.maskAlarm = 1;
      menuMask.maskFM    = 1;
      menuMask.maskMP    = 1;
      menuMask.maskBT    = 1;
    
      break;
    case PICK_TIME:
      menuMask.maskDate  = 1;
      menuMask.maskTime  = 0;
      menuMask.maskAlarm = 1;
      menuMask.maskFM    = 1;
      menuMask.maskMP    = 1;
      menuMask.maskBT    = 1;
    
      break;
    case PICK_ALARM:
      menuMask.maskDate  = 1;
      menuMask.maskTime  = 1;
      menuMask.maskAlarm = 0;
      menuMask.maskFM    = 1;
      menuMask.maskMP    = 1;
      menuMask.maskBT    = 1;
    
      break;
    case PICK_FM:
      menuMask.maskDate  = 1;
      menuMask.maskTime  = 1;
      menuMask.maskAlarm = 1;
      menuMask.maskFM    = 0;
      menuMask.maskMP    = 1;
      menuMask.maskBT    = 1;
    
      break;
    case PICK_MUSIC:
      menuMask.maskDate  = 1;
      menuMask.maskTime  = 1;
      menuMask.maskAlarm = 1;
      menuMask.maskFM    = 1;
      menuMask.maskMP    = 0;
      menuMask.maskBT    = 1;
    
      break;
    case PICK_BLUETOOTH:
      menuMask.maskDate  = 1;
      menuMask.maskTime  = 1;
      menuMask.maskAlarm = 1;
      menuMask.maskFM    = 1;
      menuMask.maskMP    = 1;
      menuMask.maskBT    = 0;
    
      break;
  }
}

/*********************************************************************************************************
* 函数名称: SetTimeFace
* 函数功能: 设置时间界面
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void SetTimeFace(void)
{
  u8 getHour = calendar.hour;
  u8 getMin  = calendar.min;
  u8 nowSetHour = 1;
  u8 nowSetMin  = 0;
  
  oneSet = 1;
  
  while(1)
  {
    ScanKeyOne(KEY_NAME_KEY0, ProcKeyUpKey0, ProcKeyDownKey0);      //扫描得到按键0的状态
    ScanKeyOne(KEY_NAME_KEY1, ProcKeyUpKey1, ProcKeyDownKey1);      //扫描得到按键1的状态
    ScanKeyOne(KEY_NAME_KEY2, ProcKeyUpKey2, ProcKeyDownKey2);      //扫描得到按键2的状态
        
    if(SetTimePressUp == 1)
    {
      SetTimePressUp = 0;
      
      if(nowSetHour)
      {
        if(getHour >= 23)
        {
          getHour = 0;
        }
        else
        {
          getHour++;
        } 
      }
      
      if(nowSetMin)
      {
        if(getMin >= 59)
        {
          getMin = 0;
        }
        else
        {
          getMin++;
        }
      }
          
    }
    
    if(SetTimePressDown == 1)
    {
      SetTimePressDown = 0;
      
      if(nowSetHour)
      {
        if(getHour <= 0)
        {
          getHour = 23;
        }
        else
        {
          getHour--;
        }
      }
      if(nowSetMin)
      {
        if(getMin <= 0)        
        {
          getMin = 59;
        }
        else
        {
          getMin--;
        }
      }
      
    }
    
    if(SetTimePressSet == 1)
    {
      nowSetHour = 0;
      nowSetMin  = 1;
    }
    
    if(oneSet == 1)
    {
      oneSet = 0;
      
      OLEDClear();
      OLEDShowNum(44, 24, getHour, 2, 16, 1 - nowSetHour);
      OLEDShowChar(60, 24, ':', 16, 1);
      OLEDShow0Num(68, 24, getMin, 2, 16, 1 - nowSetMin);
      OLEDRefreshGRAM();
    }
    
    if(SetTimePressSet == 2)
    {
      SetTimePressSet = 0;
      oneSet = 0;
      SetRTC(calendar.year, calendar.month, calendar.date, getHour, getMin, 0);
      OLEDClear();
      NowInterFace = 0;
      NowPointAt   = 5;
      UpdateMask(NowPointAt);
      ShowMenu();
      return;
    }
  }
}

/*********************************************************************************************************
* 函数名称: SetDateFace
* 函数功能: 设置日期界面
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  SetDateFace(void)
{
  u16 getYear     = calendar.year;
  u8  getMonth    = calendar.month;
  u8  getDate     = calendar.date;
  u8  nowSetYear  = 1;
  u8  nowSetMonth = 0;
  u8  nowSetDay   = 0;
  
  oneSet = 1;
  
  while(1)
  {
    ScanKeyOne(KEY_NAME_KEY0, ProcKeyUpKey0, ProcKeyDownKey0);      //扫描得到按键0的状态
    ScanKeyOne(KEY_NAME_KEY1, ProcKeyUpKey1, ProcKeyDownKey1);      //扫描得到按键1的状态
    ScanKeyOne(KEY_NAME_KEY2, ProcKeyUpKey2, ProcKeyDownKey2);      //扫描得到按键2的状态
    
    if(SetDatePressUp == 1)
    {
      SetDatePressUp = 0;
      
      if(nowSetYear)
      {
        if(getYear >= 2099)
        {
          getYear = 1970;
        }
        else
        {
          getYear++;
        }                
      }
      
      if(nowSetMonth)
      {
        if(getMonth >= 12)
        {
          getMonth = 1;
        }
        else
        {
          getMonth++;
        }
      }
      
      if(nowSetDay)
      {
        if(getMonth == 1 || getMonth == 3 || getMonth == 5 || getMonth == 7 || 
                                    getMonth == 8 || getMonth == 10 || getMonth == 12)
        {
          if(getDate >= 31)
          {
            getDate = 1;
          }
          else
          {
            getDate++;
          }
        }
        else if(getMonth == 2 && !JudgeLeapYear(getYear))
        {
          if(getDate >= 28)
          {
            getDate = 1;
          }
          else
          {
            getDate++;
          }
        }
        else if(getMonth == 2 && JudgeLeapYear(getYear))
        {
          if(getDate >= 29)
          {
            getDate = 1;
          }
          else
          {
            getDate++;
          }
        }
        else
        {
          if(getDate >= 30)
          {
            getDate = 1;
          }
          else
          {
            getDate++;
          }
        }
      }
    }

    if(SetDatePressDown == 1)
    {
      SetDatePressDown = 0;
      
      if(nowSetYear)
      {
        if(getYear <= 1970)
        {
          getYear = 2099;
        }
        else
        {
          getYear--;
        }
      }
      
      if(nowSetMonth)
      {
        if(getMonth <= 1)
        {
          getMonth = 12;
        }
        else
        {
          getMonth--;
        }
      }
      
      if(nowSetDay)
      {
        if(getMonth == 1 || getMonth == 3 || getMonth == 5 || getMonth == 7 || 
                                    getMonth == 8 || getMonth == 10 || getMonth == 12)
        {
          if(getDate <= 1)
          {
            getDate = 31;
          }
          else
          {
            getDate--;
          }
        }
        else if(getMonth == 2 && !JudgeLeapYear(getYear))
        {
          if(getDate <= 1)
          {
            getDate = 28;
          }
          else
          {
            getDate--;
          }
        }
        else if(getMonth == 2 && JudgeLeapYear(getYear))
        {
          if(getDate <= 1)
          {
            getDate = 29;
          }
          else
          {
            getDate--;
          }
        }
        else
        {
          if(getDate <= 1)
          {
            getDate = 30;
          }
          else
          {
            getDate--;
          }
        }
      }
    }
    
    if(SetDatePressSet == 1)
    {
      nowSetYear  = 0;
      nowSetMonth = 1;
      nowSetDay   = 0;
    }
    
    if(SetDatePressSet == 2)
    {
      nowSetYear  = 0;
      nowSetMonth = 0;
      nowSetDay   = 1;
    }
    
    if(oneSet == 1)
    {
      oneSet = 0;
      OLEDClear();
      OLEDShowNum(8, 24, getYear, 4, 16, 1 - nowSetYear);
      Show_Font(40, 24, "年", 16, 1);
      OLEDShowNum(56, 24, getMonth, 2, 16, 1 - nowSetMonth);
      Show_Font(72, 24, "月", 16, 1);
      OLEDShowNum(88, 24, getDate, 2, 16, 1 - nowSetDay);
      Show_Font(104, 24, "日", 16, 1);
      OLEDRefreshGRAM();
    }
    
    if(SetDatePressSet == 3)
    {
      SetDatePressSet = 0;
      oneSet = 0;
      SetRTC(getYear, getMonth, getDate, calendar.hour, calendar.min, calendar.sec);
      OLEDClear();
      NowInterFace = 0;
      NowPointAt   = 5;
      UpdateMask(NowPointAt);
      ShowMenu();
      return;
    }
  }
}

/*********************************************************************************************************
* 函数名称: SetAlarmFace
* 函数功能: 设置闹钟界面
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void SetAlarmFace(void)
{
  u8 getAlarmHour  = nowAlarmHour;
  u8 getAlarmMin   = nowAlarmMin;
  u8 getAlarmState = AlarmState;
  
  u8 nowSetAlarmState = 1;
  u8 nowSetAlarmHour  = 0;
  u8 nowSetAlarmMin   = 0;
  
  oneSet = 1;
  
  while(1)
  {
    ScanKeyOne(KEY_NAME_KEY0, ProcKeyUpKey0, ProcKeyDownKey0);      //扫描得到按键0的状态
    ScanKeyOne(KEY_NAME_KEY1, ProcKeyUpKey1, ProcKeyDownKey1);      //扫描得到按键1的状态
    ScanKeyOne(KEY_NAME_KEY2, ProcKeyUpKey2, ProcKeyDownKey2);      //扫描得到按键2的状态
    
    if(SetAlarmPressUp == 1)
    {
      SetAlarmPressUp = 0;
      
      if(nowSetAlarmState)
      {
        getAlarmState = 1 - getAlarmState;
      }
      
      if(nowSetAlarmHour)
      {
        if(getAlarmHour >= 23)
        {
          getAlarmHour = 0;
        }
        else
        {
          getAlarmHour++;
        }
      }
        
      if(nowSetAlarmMin)
      {
        if(getAlarmMin >= 59)
        {
          getAlarmMin = 0;
        }
        else
        {
          getAlarmMin++;
        }
      }
    }
    
    if(SetAlarmPressDown == 1)
    {
      SetAlarmPressDown = 0;
      
      if(nowSetAlarmState)
      {
        getAlarmState = 1 - getAlarmState;
      }
      
      if(nowSetAlarmHour)
      {
        if(getAlarmHour <= 0)
        {
          getAlarmHour = 23;
        }
        else
        {
          getAlarmHour--;
        }
      }
      
      if(nowSetAlarmMin)
      {
        if(getAlarmMin <= 0)
        {
          getAlarmMin = 59;
        }
        else
        {
          getAlarmMin--;
        }
      }
    }
    
    if(SetAlarmPressSet == 1)
    {
      if(getAlarmState == 0)
      {
        SetDatePressSet = 0;
        oneSet = 0;
        SetAlarmPressSet = 0;
        /*********************************************************************************************************
        这里要写入关闭闹钟函数,记得在其中把闹钟状态标志位改成0
        同时最好把闹钟图标改一下
*********************************************************************************************************/
        RTC_ITConfig(RTC_IT_ALR, DISABLE);
        AlarmState = 0;
        
        OLEDClear();
        NowInterFace = 0;
        NowPointAt   = 5;
        UpdateMask(NowPointAt);
        ShowMenu();
        
        return;
      }
      else if(getAlarmState == 1)
      {
        RTC_ITConfig(RTC_IT_ALR, ENABLE);
        nowSetAlarmState = 0;
        nowSetAlarmHour  = 1;
        nowSetAlarmMin   = 0;
      }
    }
    
    if(SetAlarmPressSet == 2)
    {
      nowSetAlarmState = 0;
      nowSetAlarmHour  = 0;
      nowSetAlarmMin   = 1;
    }
    
    if(oneSet == 1)
    {
      oneSet = 0;
      
      OLEDClear();
      Show_Str(36, 16, 40, 16, "闹钟：", 16, 1);
      
      if(getAlarmState)
      {
        Show_Str(76, 16, 16, 16, "开", 16, 1 - nowSetAlarmState);
        OLEDShowNum(44, 32, getAlarmHour, 2, 16, 1 - nowSetAlarmHour);
        OLEDShowChar(60, 32, ':', 16, 1);
        OLEDShow0Num(68, 32, getAlarmMin, 2, 16, 1 - nowSetAlarmMin);
      }
      else
      {
        Show_Str(76, 16, 16, 16, "关", 16, 1 - nowSetAlarmState);
      }
      
      OLEDRefreshGRAM();
    }
    
    if(SetAlarmPressSet == 3)
    {
      SetAlarmPressSet = 0;
      oneSet = 0;
      SetAlarm(getAlarmHour, getAlarmMin);
      OLEDClear();
      NowInterFace = 0;
      NowPointAt   = 5;
      UpdateMask(NowPointAt);
      ShowMenu();
      
      return;
    }
  } 
}

/*********************************************************************************************************
* 函数名称: AlarmisRing
* 函数功能: 闹钟响了
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void AlarmisRing(void)
{ 
  OLEDClear();
//  Show_Str(0, 0, 128, 16, "闹钟响了", 16, 1);
  OLEDShowAlarmRing();
  OLEDRefreshGRAM();
  
  AlarmStop = 1;
  
  while(AlarmStop)
  {
    PlaySystemMusic("0:/OTHERS/AlarmMusic.mp3");
  }
  
  NowInterFace = 0;
  NowPointAt   = 5;
  UpdateMask(NowPointAt);
  ShowMenu();
}
