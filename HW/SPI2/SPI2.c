/*********************************************************************************************************
* 模块名称: SPI2.c
* 摘    要: 
* 当前版本: 1.0.0
* 作    者: 
* 完成日期: 2018年03月01日 
* 内    容: 以下是SPI模块的初始化代码，配置成主机模式
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
#include "SPI2.h"
#include "stm32f10x.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/

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
* 函数名称: InitSPI2
* 函数功能: 初始化SPI
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void InitSPI2(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;//定义结构体GPIO_InitStructure,用来配置SPI的GPIO
  SPI_InitTypeDef  SPI_InitStructure; //定义结构SPI_InitStructure,用来配置SPI的参数

  RCC_APB1PeriphClockCmd(USER_DEFINE_SPI2_CLK,  ENABLE);           //SPI2时钟使能
 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;                  //复用推挽输出模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                //设置I/O口速率为50MHz
  
  RCC_APB2PeriphClockCmd(USER_DEFINE_SPI2_SCK_GPIO_CLK, ENABLE);   //使能SPI.SCK的时钟  
  GPIO_InitStructure.GPIO_Pin = USER_DEFINE_SPI2_SCK_GPIO_PIN;     //设置SPI.SCK的引脚
  GPIO_Init(USER_DEFINE_SPI2_SCK_GPIO_PORT, &GPIO_InitStructure);  //根据参数初始化SPI.SCK的GPIO端口

  RCC_APB2PeriphClockCmd(USER_DEFINE_SPI2_MISO_GPIO_CLK, ENABLE);  //使能SPI.MISO的时钟   
  GPIO_InitStructure.GPIO_Pin = USER_DEFINE_SPI2_MISO_GPIO_PIN;    //设置SPI.MISO的引脚
  GPIO_Init(USER_DEFINE_SPI2_MISO_GPIO_PORT, &GPIO_InitStructure); //根据参数初始化SPI.MISO的GPIO端口

  RCC_APB2PeriphClockCmd(USER_DEFINE_SPI2_MOSI_GPIO_CLK, ENABLE);  //使能SPI.MOSI的时钟 
  GPIO_InitStructure.GPIO_Pin = USER_DEFINE_SPI2_MOSI_GPIO_PIN;    //设置SPI.MOSI的引脚
  GPIO_Init(USER_DEFINE_SPI2_MOSI_GPIO_PORT, &GPIO_InitStructure); //根据参数初始化SPI.MOSI的GPIO端口

  GPIO_SetBits(USER_DEFINE_SPI2_SCK_GPIO_PORT, USER_DEFINE_SPI2_SCK_GPIO_PIN);  //初始化SPI.SCK输出高电平
  GPIO_SetBits(USER_DEFINE_SPI2_MISO_GPIO_PORT, USER_DEFINE_SPI2_MISO_GPIO_PIN);//初始化SPI.MISO输出高电平
  GPIO_SetBits(USER_DEFINE_SPI2_MOSI_GPIO_PORT, USER_DEFINE_SPI2_MOSI_GPIO_PIN);//初始化SPI.MOSI输出高电平

  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;       //设置SPI工作模式:设置为主SPI
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;   //设置SPI的数据大小:SPI发送接收8位帧结构
  SPI_InitStructure.SPI_CPOL = USER_DEFINE_SPI2_CPOL; //串行同步时钟的空闲状态为高电平
  SPI_InitStructure.SPI_CPHA = USER_DEFINE_SPI2_CPHA; //串行同步时钟的第二个跳变沿（上升或下降）数据被采样
  //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;   
  //定义波特率预分频的值:波特率预分频值为256
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;  
  //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;  
  SPI_InitStructure.SPI_CRCPolynomial = 7;            //CRC值计算的多项式
  SPI_Init(USER_DEFINE_SPI2_PORT, &SPI_InitStructure);//根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
 
  SPI_Cmd(USER_DEFINE_SPI2_PORT, ENABLE);             //使能SPI外设
  
  SPI2ReadWriteByte(0xff);                             //启动传输
}   
  
/*********************************************************************************************************
* 函数名称: SPI2SetSpeed
* 函数功能: 设置SPI的速率
* 输入参数: SPI_BaudRatePrescaler，SPI的波特率分频值
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: SPI 速度设置函数
*           SpeedSet:
*           SPI_BaudRatePrescaler_2   2分频   
*           SPI_BaudRatePrescaler_8   8分频   
*           SPI_BaudRatePrescaler_16  16分频  
*           SPI_BaudRatePrescaler_256 256分频       
*********************************************************************************************************/
void SPI2SetSpeed(u8 SPI_BaudRatePrescaler)
{
  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler)); //判断输入的参数是否正确
  
  USER_DEFINE_SPI2_PORT->CR1 &= 0XFFC7;                            //初始化GPIO速率的寄存器                     
  USER_DEFINE_SPI2_PORT->CR1 |= SPI_BaudRatePrescaler;             //设置SPI2速率 
  
  SPI_Cmd(USER_DEFINE_SPI2_PORT, ENABLE);                         //使能SPI外设
} 

/*********************************************************************************************************
* 函数名称: SPIReadWriteByte
* 函数功能: 读写一个字节 
* 输入参数: TxData:要写入的字节
* 输出参数: void
* 返 回 值: 读取到的字节
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8 SPI2ReadWriteByte(u8 TxData)
{
  u8 reTry = 0;       //重试标志位
  
  //检查指定的SPI标志位设置与否:发送缓存空标志位
  while(SPI_I2S_GetFlagStatus(USER_DEFINE_SPI2_PORT, SPI_I2S_FLAG_TXE) == RESET) 
  {
    reTry++;          //每循环一次加1
    
    if(reTry > 200)   //检查时间过长
    {
      return 0;       //读写操作失败
    }
  }

  SPI_I2S_SendData(USER_DEFINE_SPI2_PORT, TxData);     //通过外设SPIx发送一个数据
  
  reTry = 0;          //清零

  //检查指定的SPI标志位设置与否:接受缓存非空标志位
  while (SPI_I2S_GetFlagStatus(USER_DEFINE_SPI2_PORT, SPI_I2S_FLAG_RXNE) == RESET) 
  {
    reTry++;          //每循环一次加1
    
    if(reTry > 200)   //检查时间过长
    {
      return 0;       //读写操作失败
    }
  }
  
  return SPI_I2S_ReceiveData(USER_DEFINE_SPI2_PORT);   //返回通过SPIx最近接收的数据
}
