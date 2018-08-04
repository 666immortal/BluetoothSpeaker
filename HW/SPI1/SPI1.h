/*********************************************************************************************************
* 模块名称: SPI1.h
* 摘    要: SPI1驱动
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
#ifndef _SPI1_H_
#define _SPI1_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "DataType.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define USER_DEFINE_SPI1_CS_GPIO_PORT    GPIOA                   //用户定义SPI1.CS的GPIO端口
#define USER_DEFINE_SPI1_CS_GPIO_PIN     GPIO_Pin_4             //用户定义SPI1.CS的引脚
#define USER_DEFINE_SPI1_CS_GPIO_CLK     RCC_APB2Periph_GPIOB    //用户定义SPI1.CS的时钟

#define USER_DEFINE_SPI1_SCK_GPIO_PORT   GPIOA                   //用户定义SPI1.SCK的GPIO端口
#define USER_DEFINE_SPI1_SCK_GPIO_PIN    GPIO_Pin_5              //用户定义SPI1.SCK的引脚
#define USER_DEFINE_SPI1_SCK_GPIO_CLK    RCC_APB2Periph_GPIOA    //用户定义SPI1.SCK的时钟

#define USER_DEFINE_SPI1_MISO_GPIO_PORT  GPIOA                   //用户定义SPI1.MISO的GPIO端口
#define USER_DEFINE_SPI1_MISO_GPIO_PIN   GPIO_Pin_6             //用户定义SPI1.MISO的引脚
#define USER_DEFINE_SPI1_MISO_GPIO_CLK   RCC_APB2Periph_GPIOA    //用户定义SPI1.MISO的时钟

#define USER_DEFINE_SPI1_MOSI_GPIO_PORT  GPIOA                   //用户定义SPI1.MOSI的GPIO端口
#define USER_DEFINE_SPI1_MOSI_GPIO_PIN   GPIO_Pin_7             //用户定义SPI1.MOSI的引脚
#define USER_DEFINE_SPI1_MOSI_GPIO_CLK   RCC_APB2Periph_GPIOA    //用户定义SPI1.MOSI的时钟

#define USER_DEFINE_SPI1_PORT            SPI1                    //用户定义SPI1的端口
#define USER_DEFINE_SPI1_CLK             RCC_APB2Periph_SPI1     //用户定义SPI1的时钟
#define USER_DEFINE_SPI1_CPOL            SPI_CPOL_High           //用户定义SPI1的时钟极性
#define USER_DEFINE_SPI1_CPHA            SPI_CPHA_2Edge          //用户定义SPI1的时钟相位

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/ 				  	    													  
void  InitSPI1(void);                //初始化SPI1端口
void  SPI1SetSpeed(u8 SpeedSet);     //设置SPI1速度   
u8    SPI1ReadWriteByte(u8 TxData);  //SPI1总线读写一个字节
 
#endif
