/*********************************************************************************************************
* 模块名称: RTC.c
* 摘    要: 
* 当前版本: 1.0
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
#include "RTC.h" 		    
#include "SysTick.h"
#include "UI.h"
#include "UART.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
StructCalendar calendar;                                        //定义时间结构体calendar，用来存放日期和时间

const u8 monthTable[12] = {31,28,31,30,31,30,31,31,30,31,30,31};//平年的月份日期表
const u8 weekTable[12]  = {0,3,3,6,1,4,6,2,5,0,3,5};            //月修正数据表	  

u8  nowAlarmHour = 0;
u8  nowAlarmMin  = 0;
u8  AlarmState   = 0;

u8  AlarmFlag   = 0;
u32 AlarmSecCge = 0;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigRTCNVIC(void);                                //配置RCC的NVIC
static u32  Calendar2Sec(u16 year, u8 mon, u8 day, u8 hour, u8 min, u8 sec);

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: ConfigRTCNVIC
* 函数功能: 配置RTC的NVIC
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意:  
*********************************************************************************************************/
static void ConfigRTCNVIC(void)
{	
  NVIC_InitTypeDef NVIC_InitStructure; //定义结构体NVIC_InitStructure，用来配置RTC的NVIC
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;		        //RTC全局中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//先占优先级1位,从优先级3位
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	      //先占优先级0位,从优先级4位
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		        //使能该通道中断
	NVIC_Init(&NVIC_InitStructure);      //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
}

/*********************************************************************************************************
* 函数名称: Calendar2Sec
* 函数功能: 配置RTC的NVIC
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意:  
*********************************************************************************************************/
static u32  Calendar2Sec(u16 year, u8 mon, u8 day, u8 hour, u8 min,u8 sec)
{
  u16 t;
	u32 secCount = 0;
  
	if(year < 1970 || year > 2099)    //如果传入的年份超出合法年份的范围
  {
    return 1;                       //错误代码
  }
  
	for(t = 1970; t < year; t++)	    //把所有年份的秒钟相加
	{
		if(JudgeLeapYear(t))
    {
      secCount += 31622400;         //加上闰年的秒钟数
    }
		else 
    {
      secCount += 31536000;			    //加上平年的秒钟数
    }
	}
  
	mon -= 1;
  
	for(t = 0; t < mon; t++)	        //把前面月份的秒钟数相加
	{
    //月份秒钟数相加
		secCount += (u32)monthTable[t] * 86400;
    //判断是否为闰年且月份为2月
		if(JudgeLeapYear(year) && t == 1)
    {
      secCount += 86400;            //闰年2月份增加一天的秒钟数	   
    }
	}
  
	secCount += (u32)(day-1) * 86400; //把前面日期的秒钟数相加 
	secCount += (u32)hour * 3600;     //小时秒钟数
  secCount += (u32)min * 60;	      //分钟秒钟数
	secCount +=  sec;                 //最后的秒钟加上去
  
  return secCount;
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitRTC
* 函数功能: 实时时钟配置，初始化RTC时钟,同时检测时钟是否工作正常
* 输入参数: void
* 输出参数: void
* 返 回 值: 0：正常；1：异常
* 创建日期: 2018年03月01日
* 注    意: BKP->DR1用于保存是否第一次配置的设置
*********************************************************************************************************/
u8 InitRTC(void)
{
	u8 temp = 0;                                  //检查次数标志位
  
  //检查是不是第一次配置时钟
	if (BKP_ReadBackupRegister(BKP_DR1) != 0x5050)//从指定的后备寄存器中读出数据:读出了与写入的指定数据不相
  {	 			
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//使能PWR和BKP外设时钟   
		PWR_BackupAccessCmd(ENABLE);	            //使能后备寄存器访问 
		BKP_DeInit();	                            //复位备份区域 	
		RCC_LSEConfig(RCC_LSE_ON);	              //设置外部低速晶振(LSE),使用外设低速晶振
    
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
		{
      temp++;                                 //每检查一次，检测次数标志位加1
      DelayNms(10);                           //延时10ms
		}
    
		if(temp >= 250)                           //如果检查指定的RCC标志位的次数过多
    {
      return 1;                               //初始化时钟失败,晶振有问题	    
    }
    
		RCC_RTCCLKConfig(USER_DEFINE_RTC_CLK);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    
		RCC_RTCCLKCmd(ENABLE);	                  //使能RTC时钟  
		RTC_WaitForLastTask();	                  //等待最近一次对RTC寄存器的写操作完成
		RTC_WaitForSynchro();		                  //等待RTC寄存器同步  
    //使能RTC秒中断和RTC闹钟中断
		RTC_ITConfig(RTC_IT_SEC | RTC_IT_ALR, ENABLE);		      
		RTC_WaitForLastTask();	                  //等待最近一次对RTC寄存器的写操作完成
		RTC_EnterConfigMode();                    //允许配置	
		RTC_SetPrescaler(32767);                  //设置RTC预分频的值
		RTC_WaitForLastTask();	                  //等待最近一次对RTC寄存器的写操作完成
    SetRTC(2018, 4, 23, 21, 49, 50);           //设置RTC初始时间
		RTC_ExitConfigMode();                     //退出配置模式  
		BKP_WriteBackupRegister(BKP_DR1, 0x5050);	//向指定的后备寄存器中写入用户程序数据
	}
	else                                        //系统继续计时
	{
    RTC_WaitForSynchro();	                    //等待最近一次对RTC寄存器的写操作完成
    //使能RTC秒中断和RTC闹钟中断
		RTC_ITConfig(RTC_IT_SEC | RTC_IT_ALR, ENABLE);	
    RTC_WaitForLastTask();	                  //等待最近一次对RTC寄存器的写操作完成
	}
  
	ConfigRTCNVIC();                            //RCT中断分组设置		    				     
	GetRTC();                                   //更新时间	
  
	return 0;                                   //配置正常，时钟可以正常工作
}		 	

/*********************************************************************************************************
* 函数名称: RTC_IRQHandler
* 函数功能: RTC时钟中断，每秒触发一次 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void RTC_IRQHandler(void)
{		  
	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)     //检查秒中断是否发生
	{							
		GetRTC();                                   //更新时间   
 	} 
	
  if(RTC_GetITStatus(RTC_IT_ALR) != RESET)
  {    
    RTC_ClearITPendingBit(RTC_IT_ALR);
    
    GetRTC();
    AlarmFlag = 1;
  }
  
	RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_OW);            //清除秒中断和溢出中断
	RTC_WaitForLastTask();	  	                              //等待最近一次对RTC寄存器的写操作完成   

//  if(AlarmFlag == 1)
//  {
//    AlarmisRing();
//    AlarmSecCge = AlarmSecCge + 86400;
//    RTC_SetAlarm(AlarmSecCge);
//    AlarmFlag = 0;
//  }
}

/*********************************************************************************************************
* 函数名称: JudgeLeapYear
* 函数功能: 判断是否为闰年
* 输入参数: 年份
* 输出参数: void
* 返 回 值: 该年份是不是闰年.1：是.0：不是
* 创建日期: 2018年03月01日
* 注    意: //月份   1  2  3  4  5  6  7  8  9  10 11 12
            //闰年   31 29 31 30 31 30 31 31 30 31 30 31
            //非闰年 31 28 31 30 31 30 31 31 30 31 30 31
*********************************************************************************************************/
u8 JudgeLeapYear(u16 year)
{			  
	if(year % 4 == 0)       //必须能被4整除
	{ 
		if(year % 100 == 0)   //判断是不是以00结尾
		{ 
			if(year % 400 == 0) //如果是以00结尾，判断能不能被400整除
      {
        return 1;         //如果以00结尾,还要能被400整除，该年份才是闰年
      }
			else 
      {
        return 0;         //以00结尾但不能被400整除，该年份不是闰年
      }
		}
    else 
    {
      return 1;           //不是以00结尾而且能被4整除，该年份是闰年 
    }
	}
  else 
  {
    return 0;	            //不能被4整除，该年份不是闰年
  }
}

/*********************************************************************************************************
* 函数名称: SetRTC
* 函数功能: 设置时钟，把输入的时钟转换为秒钟，以1970年1月1日为基准
* 输入参数: void
* 输出参数: void
* 返 回 值: 0,成功;其他:错误代码.
* 创建日期: 2018年03月01日
* 注    意: 1970~2099年为合法年份
*********************************************************************************************************/							 
u8 SetRTC(u16 year, u8 mon, u8 day, u8 hour, u8 min,u8 sec)
{
	u32 secCount = 0;
  
  calendar.year  = year;
  calendar.month = mon;
  calendar.date  = day;
  calendar.hour  = hour;
  calendar.min   = min;
  calendar.sec   = sec;
  
	secCount = Calendar2Sec(year, mon, day, hour, min, sec);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟  
	PWR_BackupAccessCmd(ENABLE);	    //使能RTC和后备寄存器访问 
	RTC_SetCounter(secCount);	        //设置RTC计数器的值
	RTC_WaitForLastTask();	          //等待最近一次对RTC寄存器的写操作完成 
 	
  BKP_WriteBackupRegister(BKP_DR1, 0X5050);	//向指定的后备寄存器中写入用户程序数据
  
	return 0;	                        //成功  
}

/*********************************************************************************************************
* 函数名称: GetRTC
* 函数功能: 得到当前的时间
* 输入参数: void
* 输出参数: void
* 返 回 值: 0,成功;其他:错误代码.
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8 GetRTC(void)
{
  static u16  dayCnt    = 0;  //天数标记位
	u32         timeCount = 0;  //用来存放总的秒数
	u32         temp      = 0;  //用来存放天数
	u16         temp1     = 0;	//用来存放年份，月份
  
  timeCount  =   RTC->CNTH;   //得到计数器寄存器高位
  timeCount <<= 16;           //左移16位，空出低16位
  timeCount +=  RTC->CNTL;	  //得到计数器寄存器低位并与高位相加

 	temp = timeCount / 86400;   //得到天数(秒数对应的)
  
	if(dayCnt != temp)          //如果时间已经过了一天
	{	  
		dayCnt = temp;            //将天数赋给daycnt
    
		temp1  = 1970;	          //从1970年开始
    
		while(temp >= 365)        //如果天数大于365
		{				 
			if(JudgeLeapYear(temp1)) //如果判断年份是闰年
			{
				if(temp >= 366)       //如果天数大于366天
        {
          temp -= 366;        //天数减去366天
        }
				else                  //如果天数不大于366天
        {
          temp1++;            //年份加1             
          break;              //跳出循环
        }
			}
			else                    //如果不是闰年
      {
        temp -= 365;	        //天数减去365天
      }
			temp1++;                //天数大于365天，年份加1，进入下一年
		} 
    
		calendar.year = temp1;    //得到年份
    
		temp1 = 0;                //计算完年份后清零，用于计算月份 
    
		while(temp >= 28)         //天数超过了一个月
		{
      //判断当年是不是闰年且月份为2月
			if(JudgeLeapYear(calendar.year) && temp1 == 1)
			{
				if(temp >= 29)        //如果是闰年且月份为2月，天数大于29
        {
          temp -= 29;         //天数减去29
        }
				else                  //如果天数不大于29
        {
          break;              //跳出循环
        }
			}
			else                    //不是闰年或者不为2月
			{
        //如果总天数大于当月的天数
				if(temp >= monthTable[temp1])  
        {
          //总天数减去当月的天数
          temp -= monthTable[temp1];
        }
				else                  //如果天数不大于当月的天数
        {
          break;              //跳出循环
        }
			}
      
			temp1++;                //天数大于当月天数，月份数加1，进入下个月
		}
    
		calendar.month = temp1 + 1;	        //得到月份
		calendar.date  = temp  + 1;  	      //得到日期 
	}
  
	temp = timeCount % 86400;     		    //得到剩余的秒数   	   
	calendar.hour = temp / 3600;     	    //得到当前小时
	calendar.min  = (temp % 3600) / 60; 	//得到当前分钟	
	calendar.sec  = (temp % 3600) % 60; 	//得到当前秒钟
	calendar.week = GetRTCWeek(calendar.year, calendar.month, calendar.date);//获取星期
  
	return 0;                             //成功
}	 

/*********************************************************************************************************
* 函数名称: GetRTCWeek
* 函数功能: 获得现在是星期几，功能描述:输入公历日期得到星期(只允许1901-2099年)
* 输入参数: 公历年月日 
* 输出参数: void
* 返 回 值: 星期号	
* 创建日期: 2018年03月01日
* 注    意: 此处用蔡勒公式来计算星期，返回0为Sunday
*********************************************************************************************************/																			 
u8 GetRTCWeek(u16 year, u8 month, u8 day)
{	
	u16 temp2;
	u8  yearH;          //用于存放年份的前两位
  u8  yearL;          //用于存放年份的后两位
	
	yearH = year / 100;	//取出年份的前两位
  yearL = year % 100; //取出年份的后两位
	
	if (yearH > 19)     //如果为21世纪 
  {
    yearL += 100;     //年份的后两位数加100
  }
	//所过闰年数只算1900年之后的，根据蔡勒算法来计算星期
	temp2 = yearL + yearL / 4;  
	temp2 = temp2 % 7;  
	temp2 = temp2 + day + weekTable[month - 1];
  
	if (yearL % 4 == 0 && month < 3) 
  {
    temp2--;          //如果是1月或者2月，temp2要减1
  }
  
	return(temp2 % 7);  //返回星期号
}			  

/*********************************************************************************************************
* 函数名称: SetAlarm
* 函数功能: 设置闹钟的时间
* 输入参数: 公历年月日 
* 输出参数: void
* 返 回 值: 星期号	
* 创建日期: 2018年03月01日
* 注    意: 此处用蔡勒公式来计算星期，返回0为Sunday
*********************************************************************************************************/			
u8 SetAlarm(u8 hour, u8 min)
{
  u32 secCount = 0;
  
  AlarmState   = 1;
  nowAlarmHour = hour;
  nowAlarmMin  = min;
  
  if(hour >= calendar.hour && min > calendar.min)
  {
    secCount = Calendar2Sec(calendar.year, calendar.month, calendar.date,
                                  hour, min, 0);
    AlarmSecCge = secCount;
    
    //不止是SetAlarm()，所有针对RTC寄存器的设置都会出错，因为RTC在备份域、寄存器是挂在APB1总线上的，所以访
    //问RTC寄存器需要2个先决条件：
    //1）APB1时钟使能
    //2）允许访问BKP区域，就是PWR_CR的 DPB 位需要置1
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//使能PWR和BKP外设时钟   
		PWR_BackupAccessCmd(ENABLE);	            //使能后备寄存器访问 
    RTC_SetAlarm(secCount);
    RTC_WaitForLastTask();
  }
  else
  {
    secCount = Calendar2Sec(calendar.year, calendar.month, calendar.date + 1,
                                  hour, min, 0);
    AlarmSecCge = secCount;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//使能PWR和BKP外设时钟   
		PWR_BackupAccessCmd(ENABLE);	            //使能后备寄存器访问 
    RTC_SetAlarm(secCount);
    RTC_WaitForLastTask();
  }
  
  BKP_WriteBackupRegister(BKP_DR1, 0X5050);	//向指定的后备寄存器中写入用户程序数据
  
  return 0;
}
