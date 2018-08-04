/*********************************************************************************************************
* 模块名称: SPI2.h
* 摘    要: SPI2驱动
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
#ifndef _SPI2_H_
#define _SPI2_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "DataType.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define USER_DEFINE_SPI2_CS_GPIO_PORT    GPIOB                   //用户定义SPI2.CS的GPIO端口
#define USER_DEFINE_SPI2_CS_GPIO_PIN     GPIO_Pin_12             //用户定义SPI2.CS的引脚
#define USER_DEFINE_SPI2_CS_GPIO_CLK     RCC_APB2Periph_GPIOB    //用户定义SPI2.CS的时钟
                                                                             
#define USER_DEFINE_SPI2_SCK_GPIO_PORT   GPIOB                   //用户定义SPI2.SCK的GPIO端口
#define USER_DEFINE_SPI2_SCK_GPIO_PIN    GPIO_Pin_13             //用户定义SPI2.SCK的引脚
#define USER_DEFINE_SPI2_SCK_GPIO_CLK    RCC_APB2Periph_GPIOB    //用户定义SPI2.SCK的时钟
                                                                            
#define USER_DEFINE_SPI2_MISO_GPIO_PORT  GPIOB                   //用户定义SPI2.MISO的GPIO端口
#define USER_DEFINE_SPI2_MISO_GPIO_PIN   GPIO_Pin_14             //用户定义SPI2.MISO的引脚
#define USER_DEFINE_SPI2_MISO_GPIO_CLK   RCC_APB2Periph_GPIOB    //用户定义SPI2.MISO的时钟
                                                                             
#define USER_DEFINE_SPI2_MOSI_GPIO_PORT  GPIOB                   //用户定义SPI2.MOSI的GPIO端口
#define USER_DEFINE_SPI2_MOSI_GPIO_PIN   GPIO_Pin_15             //用户定义SPI2.MOSI的引脚
#define USER_DEFINE_SPI2_MOSI_GPIO_CLK   RCC_APB2Periph_GPIOB    //用户定义SPI2.MOSI的时钟
                                                                             
#define USER_DEFINE_SPI2_PORT            SPI2                    //用户定义SPI2的端口
#define USER_DEFINE_SPI2_CLK             RCC_APB1Periph_SPI2     //用户定义SPI2的时钟
#define USER_DEFINE_SPI2_CPOL            SPI_CPOL_High           //用户定义SPI2的时钟极性
#define USER_DEFINE_SPI2_CPHA            SPI_CPHA_2Edge          //用户定义SPI2的时钟相位

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/ 				  	    													  
void  InitSPI2(void);                //初始化SPI2端口
void  SPI2SetSpeed(u8 SpeedSet);     //设置SPI2速度   
u8    SPI2ReadWriteByte(u8 TxData);  //SPI2总线读写一个字节
 
#endif
