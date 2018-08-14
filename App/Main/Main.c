/*********************************************************************************************************
* 模块名称: Main.c
* 摘    要: 主文件
* 当前版本: 1.0.0
* 作    者: 
* 完成日期: 2018年03月01日
* 内    容:
* 注    意: 注意勾选Options for Target 'Target1'->Code Generation->Use MicroLIB                                                                  
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
#include <stm32f10x_conf.h>
#include <string.h>
#include "Main.h"
#include "DataType.h"
#include "Timer.h"        
#include "UART.h"
#include "NVIC.h"
#include "RCC.h"
#include "LED.h"
#include "SysTick.h"
#include "Malloc.h"
#include "RTC.h"
#include "ADC.h"
#include "KeyOne.h"
#include "ProcKeyOne.h"
#include "TouchKey.h"
#include "Lamp.h"
#include "ff.h"
#include "W25Qxx.h"
#include "ExFsFun.h"
#include "DbgFat.h"
#include "CheckLineFeed.h"
#include "DbgAsst.h"	
#include "OLED.h"
#include "FontUpd.h"
#include "Text.h"
#include "WakeUp.h"
#include "VS10xx.h"
#include "MusicPlayer.h"
#include "UI.h"
#include "AudioSel.h"
#include "UART2.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
  
/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/                                             
const u8 openSPP[] = {0x41, 0x50, 0x54, 0x2B, 0x53, 0x50, 0x50, 0x38, 0x38, 0x38, 0x38, 0x0D, 0x0A};
const u8 tip[] = "Error\r\n";
u8 NowInterFace = 0;      //界面
u8 NowPointAt   = 5;      //光标
u8  initBTFlag  = 0;      //初始化蓝牙标志

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static  void  InitSoftware(void);           //初始化软件相关函数
static  void  InitHardware(void);           //初始化硬件相关函数
static  void  Proc2msTask(void);            //处理2ms任务
static  void  Proc100msTask(void);          //处理100ms任务
static  void  Proc1SecTask(void);           //处理1s任务

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitSoftware
* 函数功能: 所有的软件初始化函数都放在此函数中
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  void  InitSoftware(void)
{
  InitProcKeyOne();               //初始化按键操作
  InitCheckLineFeed();            //初始化CheckLineFeed
  InitDbgAsst();                  //初始化DbgAsst 
  InitMemMgt(USER_DEFINE_SRAMIN); //初始化内存管理
  InitExFsFun();                  //为文件系统变量申请内存
  InitDbgFat();                   //初始化文件系统调试程序
}

/*********************************************************************************************************
* 函数名称: InitHardware
* 函数功能: 所有的硬件初始化函数都放在此函数中
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  void  InitHardware(void)
{  
  SystemInit();       //初始化系统
  InitRCC();          //初始化RCC
  InitNVIC();         //初始化中断函数
  InitUART();         //初始化串口
  InitTimer();        //初始化定时器
  InitADC();          //初始化ADC
  InitLED();          //初始化LED
  InitSysTick();      //初始化SysTick
  InitWakeUp();       //初始化待机唤醒
  InitRTC();          //初始化RTC实时时钟
  InitAudioSel();     //初始化音频选择器
  InitKeyOne();       //初始化KeyOne
  InitTouchKey();     //初始化触摸按键
  InitLamp();         //初始化RGB彩灯
  InitFatFs();        //初始化文件系统
  InitOLED();         //初始化OLED
  VS_Init();          //初始化VS1053
  InitUART2();        //初始化UART2 
}

/*********************************************************************************************************
* 函数名称: Proc2msTask
* 函数功能: 处理2ms任务 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 用串口助手发送do指令来调用函数检查SD卡
*********************************************************************************************************/
static  void  Proc2msTask(void)
{    
  static u8 s_iCnt100   = 0;        //200ms计时器
  static u8 prevPointAt = 0;        //之前指向的标签
      
  if(Get2msFlag())                              //检查2ms标志状态   
  {                         
    if(AlarmFlag == 1)                          //闹钟时间到了
    {
      NowInterFace = 5;                         //进入闹钟界面
      AlarmisRing();                            //播放闹铃并在其中循环直至退出闹钟
      AlarmSecCge = AlarmSecCge + 86400;        //退出闹钟后设置下一次闹钟为第二天同一时间
      RTC_SetAlarm(AlarmSecCge);                //实现设置闹钟
      AlarmFlag = 0;                            //闹钟标志置为0
    }
        
    ScanKeyOne(KEY_NAME_KEY0, ProcKeyUpKey0, ProcKeyDownKey0);      //扫描得到按键0的状态
    ScanKeyOne(KEY_NAME_KEY1, ProcKeyUpKey1, ProcKeyDownKey1);      //扫描得到按键1的状态
    ScanKeyOne(KEY_NAME_KEY2, ProcKeyUpKey2, ProcKeyDownKey2);      //扫描得到按键2的状态
    
    if(s_iCnt100 >= 100)                        //计时器溢出
    {
      if(NowInterFace == MENU)                  //处在菜单界面
      {   
        if(prevPointAt != NowPointAt)           //光标位置发生了改变
        {
          prevPointAt = NowPointAt;             //存储当前光标位置
          UpdateMask(NowPointAt);               //更新光标清单
          ShowMenu();                           //更新OLED屏
        }
      }
      s_iCnt100 = 0;                            //计时器置0
    }
    else
    {
      s_iCnt100++;                              //计时器加1
    }
    
    if(NowInterFace == MUSIC)                   //处在音乐界面
    {
      OperateMusicPlayer();                     //播放音乐操作函数
    }
    
    if(NowInterFace == SET_DATE)                //处在设置日期界面
    {
      SetDateFace();                            //设置日期界面函数
    }
    
    if(NowInterFace == SET_TIME)                //处在设置时间界面
    {
      SetTimeFace();                            //设置时间界面函数
    }
    
    if(NowInterFace == SET_ALARM)               //处在设置闹钟界面
    {
      SetAlarmFace();                           //设置闹钟界面函数
    }
            
    Clr2msFlag();                               //清除2ms标志
  }    
}

/*********************************************************************************************************
* 函数名称: Proc100msTask
* 函数功能: 处理100ms任务 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  void  Proc100msTask(void)
{
  if(Get100msFlag())//检查100ms标志状态
  {
    DbgAsstScan();  //扫描函数,用于处理从串口接收来的数据
    
    Clr100msFlag(); //清除100ms标志
  }    
}

/*********************************************************************************************************
* 函数名称: Proc1SecTask
* 函数功能: 处理1sec任务 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  void  Proc1SecTask(void)
{  
  u16    ADCNum  = 0;               //存放ADC检测到的数据
  float  ADCVolt = 0;               //存放ADC数据转化成电压的值
  u8* getCode;
  static u8 i = 0;
  static u8 iHadReceive  = 0;
  static  u8 s_iCnt30    = 0;       //30s计时器
  static  u8 upDate      = 0;       //日期更新标志
  static  u8 upTime      = 0;       //时间更新标志
    
  if(Get1SecFlag())                 //检查1s标志状态
  {   
    if(s_iCnt30 >= 30)              //30s计时器溢出
    {   
      if(NowInterFace == MENU)      //处在菜单界面
      { 
        if(upTime != calendar.min)  //检查秒是否发生更新
        { 
          upTime = calendar.min;    //将更新的秒值赋给update
          ShowMenu();               //更新OLED界面
        }
        
        if(upDate != calendar.date) //检查日期是否发生更新
        {
          upDate = calendar.date;   //将更新的日期赋给upDate
          ShowMenu();               //更新OLED界面
        }
      }
      s_iCnt30 = 0;                 //计时器清零
      
      ADCNum  = GetADCAverage(10);  //获取ADC数据
      ADCVolt = (float)ADCNum * (3.3 / 4096); //将ADC数据转化为电压值
      printf("The volt is: %f\r\n", ADCVolt); //打印ADC电压值
    }
    else
    {
      s_iCnt30++;                   //计时器加1
    }
    
    if(initBTFlag == 0)
    { 
      iHadReceive = 0;
      
      if(WaitForReceive((u8*)"II"))
      {
        i = 0;
        
        do
        {
          WriteUART2((u8 *)openSPP, sizeof(openSPP));
          i++;
          DelayNms(1000);
        }while((iHadReceive = WaitForReceive((u8*)"OK")) == 0 && i <= 10);
      }
      
      if(i > 10)
      {
        printf("Error\r\n");
      }
      if(iHadReceive == 1)
      {
        initBTFlag = 1;
      }
    }
    else if(initBTFlag == 1)
    {
      WaitForTask(getCode);
    }
        
    Clr1SecFlag();                  //清除1s标志
  }    
}

/*********************************************************************************************************
* 函数名称: main
* 函数功能: 主函数 
* 输入参数: void
* 输出参数: void
* 返 回 值: int
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
int main(void)
{ 
//  u16 type = 0;
//  u32 all  = 0;
//  u32 left = 0;
  u8 status = 0;    //获取更新字库结果
  
  InitSoftware();                                //初始化软件相关函数
  InitHardware();                                //初始化硬件相关函数
  
  if(InitFontLib())                              //检测字库是否需要更新
  {
    status = update_font(0, 0, 16, "0:");        //从SD卡字库文件更新字库
  }
  else
  {
    OLEDShowString(0, 32, "FLib exited!", 12, 1);//屏幕显示字库存在
    OLEDRefreshGRAM();                           //更新GRAM
  }
  
  if(status)                                          //如果更新字库失败
  {
    OLEDShowString(0, 44, "Update Failed!" ,12, 1);   //屏幕显示更新字库失败
    OLEDRefreshGRAM();                                //更新GRAM
  }
  else 
  {
    OLEDShowString(0, 44, "Update Success!", 12, 1);  //屏幕显示更新字库成功
    OLEDRefreshGRAM();                                //更新GRAM
  }
  
  InitUI();                                           //初始化UI
  
  PlaySystemMusic("0:/OTHERS/Startup.mp3");           //播放开机音乐
  DelayNms(1000);                                     //延时1s
//  TestFatFs();                                      //测试文件系统
//  type = f_typetell((u8*)"Test3.txt");
//  printf("file type :%d", type);
//  exf_getfree((u8 *)"0:", &all, &left);
//  printf("SDcard all: %d, left: %d", all, left);   
  ShowMenu();                                         //显示菜单   
	
  while(1)                            
  {                           
    Proc2msTask();                               //处理2ms任务
    Proc100msTask();                             //处理100ms任务
    Proc1SecTask();                              //处理1s任务
  }
}
