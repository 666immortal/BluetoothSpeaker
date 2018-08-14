/*********************************************************************************************************
* 模块名称: OLED.h
* 摘    要: OLED驱动 
* 当前版本: 1.0.0
* 作    者: 
* 完成日期: 2018年03月01日 
* 内    容:
* 注    意: OLED取模使用的是PCtoLCD2002完美版软件                                                                  
**********************************************************************************************************
* 取代版本:
* 作    者:
* 完成日期: 
* 修改内容:
* 修改文件: 
*********************************************************************************************************/
#ifndef _OLED_H_
#define _OLED_H_
 
/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include  "DataType.h" 

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/ 
#define USER_DEFINE_OLED_CS_GPIO_PORT    GPIOC                     //用户定义OLED.CS的端口
#define USER_DEFINE_OLED_CS_GPIO_PIN     GPIO_Pin_0                //用户定义OLED.CS的引脚
#define USER_DEFINE_OLED_CS_GPIO_CLK     RCC_APB2Periph_GPIOC      //用户定义OLED.CS的时钟
                                         
#define USER_DEFINE_OLED_RES_GPIO_PORT   GPIOC                     //用户定义OLED.RES的端口
#define USER_DEFINE_OLED_RES_GPIO_PIN    GPIO_Pin_1                //用户定义OLED.RES的引脚
#define USER_DEFINE_OLED_RES_GPIO_CLK    RCC_APB2Periph_GPIOC      //用户定义OLED.RES的时钟
                                         
#define USER_DEFINE_OLED_DC_GPIO_PORT    GPIOC                     //用户定义OLED.DC的端口
#define USER_DEFINE_OLED_DC_GPIO_PIN     GPIO_Pin_3                //用户定义OLED.DC的引脚
#define USER_DEFINE_OLED_DC_GPIO_CLK     RCC_APB2Periph_GPIOC      //用户定义OLED.DC的时钟
                                         
#define USER_DEFINE_OLED_SCLK_GPIO_PORT  GPIOC                     //用户定义OLED.SCLK的端口
#define USER_DEFINE_OLED_SCLK_GPIO_PIN   GPIO_Pin_2                //用户定义OLED.SCLK的引脚
#define USER_DEFINE_OLED_SCLK_GPIO_CLK   RCC_APB2Periph_GPIOC      //用户定义OLED.SCLK的时钟
                                         
#define USER_DEFINE_OLED_SDIN_GPIO_PORT  GPIOC                     //用户定义OLED.SDIN的端口
#define USER_DEFINE_OLED_SDIN_GPIO_PIN   GPIO_Pin_4                //用户定义OLED.SDIN的引脚
#define USER_DEFINE_OLED_SDIN_GPIO_CLK   RCC_APB2Periph_GPIOC      //用户定义OLED.SDIN的时钟

//UI Icon
#define B_FM          0
#define B_MUSIC       1
#define B_BLUETOOTH   2
#define S_BATTERY_0   3
#define S_BATTERY_1   4
#define S_BATTERY_2   5
#define S_BATTERY_3   6
#define S_ALARM       7
#define S_ALARM_CLOSE 8

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/	    
void  InitOLED(void);           //初始化OLED
void  OLEDDisplayOn(void);      //开启OLED显示
void  OLEDDisplayOff(void);     //关闭OLED显示
void  OLEDRefreshGRAM(void);    //更新OLED GRAM

void  OLEDClear(void);                                              //清除OLED屏内容
void  OLEDDrawPoint(u8 x, u8 y, u8 t);                              //在OLED屏指定位置画点
void  OLEDShow0Num(u8 x, u8 y, u32 num, u8 len, u8 size, u8 mode);  //在OLED屏上指定位置显示带高位0的数字
void  OLEDShowNum(u8 x, u8 y, u32 num, u8 len, u8 size, u8 mode);   //在OLED屏上指定位置显示数字
void  OLEDShowChar(u8 x, u8 y, u8 chr, u8 size, u8 mode);           //在OLED屏上指定位置显示字符
void  OLEDShowString(u8 x, u8 y, const u8* p, u8 size, u8 mode);    //在OLED屏上指定位置显示字符串
void  OLEDClearArea(u8 x1, u8 y1, u8 x2, u8 y2);                    //清除屏幕上指定的区域
void  OLEDShowIcon(u8 x, u8 y, u8 object, u8 mode);                 //在指定区域显示图标
void  OLEDShowAlarmRing(void);                                      //OLED显示闹钟响了

#endif  
