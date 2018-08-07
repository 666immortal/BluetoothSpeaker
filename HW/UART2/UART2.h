/*********************************************************************************************************
* 模块名称: UART2.h
* 摘    要: UART2驱动
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
#ifndef _UART2_H_
#define _UART2_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include <stdio.h>
#include "DataType.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define UART2_BUF_SIZE 100   //设置缓冲区的大小

#define USER_DEFINE_USART2                        USART2                        //用户定义USART的端口  
#define USER_DEFINE_USART2_CLK                    RCC_APB1Periph_USART2         //用户定义USART的时钟 
#define USER_DEFINE_USART2_BAUDRATE               9600                          //用户定义USART的波特率
#define USER_DEFINE_USART2_WORDLENGTH             USART_WordLength_8b           //用户定义USART传输的字长
#define USER_DEFINE_USART2_STOPBITS               USART_StopBits_1              //用户定义USART的停止位
#define USER_DEFINE_USART2_PARITY                 USART_Parity_No               //用户定义USART的奇偶校验位
#define USER_DEFINE_USART2_MODE                   USART_Mode_Rx | USART_Mode_Tx //用户定义USART的模式
#define USER_DEFINE_USART2_HRDWAREFLOWCONTROL     USART_HardwareFlowControl_None//用户定义USART的硬件流控制
                         
#define USER_DEFINE_USART2_TX_GPIO_PORT           GPIOA                   //用户定义USART.TX的GPIO端口
#define USER_DEFINE_USART2_TX_GPIO_PIN            GPIO_Pin_2              //用户定义USART.TX的引脚
#define USER_DEFINE_USART2_TX_GPIO_CLK            RCC_APB2Periph_GPIOA    //用户定义USART.TX的时钟
#define USER_DEFINE_USART2_TX_MODE                GPIO_Mode_AF_PP         //用户定义USART.TX的模式
#define USER_DEFINE_USART2_TX_SPEED               GPIO_Speed_50MHz        //用户定义USART.TX的I/O口速率
                         
#define USER_DEFINE_USART2_RX_GPIO_PORT           GPIOA                   //用户定义USART.RX的GPIO端口 
#define USER_DEFINE_USART2_RX_GPIO_PIN            GPIO_Pin_3              //用户定义USART.RX的引脚
#define USER_DEFINE_USART2_RX_GPIO_CLK            RCC_APB2Periph_GPIOA    //用户定义USART.RX的时钟
#define USER_DEFINE_USART2_RX_MODE                GPIO_Mode_IN_FLOATING   //用户定义USART.RX的模式
#define USER_DEFINE_USART2_RX_SPEED               GPIO_Speed_50MHz        //用户定义USART.RX的I/O口速率
                         
#define USER_DEFINE_USART2_IRQHandler             USART2_IRQHandler       //用户定义串口的中断服务函数
#define USER_DEFINE_USART2_IRQ                    USART2_IRQn             //用户定义串口的中断通道

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/
void  InitUART2(void);               //初始化驱动，缺省打开串口
u8    WriteUART2(u8* pBuf, u8 len);  //写串口，返回成功写入的数据长度
u8    ReadUART2(u8* pBuf, u8 len);   //读串口，返回读到的数据长度
u8    WaitForReceive(const u8* cmd);
u8    WaitForTask(u8* receiveTask);

#endif
