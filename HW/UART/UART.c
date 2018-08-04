/*********************************************************************************************************
* 模块名称: UART.c
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
#include "UART.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              结构体枚举定义
*********************************************************************************************************/
//工作模式 
typedef enum
{
  UART_STATE_OFF,         //USART.TX状态为关闭
	UART_STATE_ON,          //USART.TX状态为打开
	UART_STATE_MAX          
}EnumUARTState;           

//串口缓冲区定义
typedef struct
{
  u8  dataBuf[UART_BUF_SIZE];   //数据缓冲区
  u8  readPtr;                  //读指针
  u8  writePtr;                 //写指针
  u8  dataLength;               //数据长度
}StructUartBuf;                 

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static  StructUartBuf s_structUARTSendBuf;  //发送串口buf
static  StructUartBuf s_structUARTRecBuf;   //接收串口buf
static  u8 s_iUARTTxSts;                    //USART.TX状态标志

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static  void  InitUARTBuf(void);            //初始化串口缓冲区，包括发送缓冲区和接收缓冲区 
static  u8    WriteReceiveBuf(u8 d);        //将接收到的数据写入到接收缓冲区中
static  u8    ReadSendBuf(u8* p);           //读取发送缓冲区中的数据

static  void  ConfigUART(void);             //配置UART的参数，包括GPIO、RCC、USART常规参数和NVIC 
static  void  EnableUARTTx(void);           //使能串口发送，在WriteUart中调用，即每次发送数据之后需要调用
                                            //打开TXE的中断开关

static  void  SendCharUsedByFputc(u16 ch);  //发送字符函数，专由fputc函数调用
  
/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitUARTBuf
* 函数功能: 初始化串口缓冲区，包括发送缓冲区和接收缓冲区  
* 输入参数: void
* 输出参数: void
* 返 回 值: void 
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  void  InitUARTBuf(void)
{
  s_structUARTSendBuf.dataLength = 0;  //初始化串口发送缓冲区数据长度
  s_structUARTSendBuf.readPtr    = 0;  //初始化读指针
  s_structUARTSendBuf.writePtr   = 0;  //初始化写指针
  
  s_structUARTRecBuf.dataLength  = 0;  //初始化串口接收缓冲区数据长度
  s_structUARTRecBuf.readPtr     = 0;  //初始化读指针
  s_structUARTRecBuf.writePtr    = 0;  //初始化写指针
}

/*********************************************************************************************************
* 函数名称: WriteReceiveBuf
* 函数功能: 写数据到串口接收BUF，在中断服务函数中，当接收到数据时，将接收到的数据写入到串口接收BUF中 
* 输入参数: d，串口中断服务函数接收到的数据
* 输出参数: void
* 返 回 值: 写入数据成功标志，0-不成功，1-成功 
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  u8  WriteReceiveBuf(u8 d)
{
  u8 ok = 0;                                                      //写入数据成功标志，0-不成功，1-成功

  if(s_structUARTRecBuf.dataLength < UART_BUF_SIZE)               //确保接收缓冲区有空间可写
  {
    ok = 1;                                                       //将标志设为1，表示写入数据成功
    s_structUARTRecBuf.dataBuf[s_structUARTRecBuf.writePtr] = d;  //写数据到接收缓冲区
    s_structUARTRecBuf.writePtr++;                                //写指针加1        

    if(s_structUARTRecBuf.writePtr >= UART_BUF_SIZE)              //如果写指针大于或等于接收缓冲区的长度
    {
      s_structUARTRecBuf.writePtr = 0;                            //写指针清零
    }
    
    s_structUARTRecBuf.dataLength++;                              //写入一个数据之后，接收缓冲区数据长度加1
  }
  
  return ok;                                                      //返回写入数据成功标志，0-不成功，1-成功 
}

/*********************************************************************************************************
* 函数名称: ReadSendBuf
* 函数功能: 读取发送BUF，用户通过WriteUart将要写入的数据传至发送缓冲，中断函数通过接收缓冲读出 
* 输入参数: void
* 输出参数: p，读出来的数据存放的首地址
* 返 回 值: 读取数据成功标志，0-不成功，1-成功 
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  u8  ReadSendBuf(u8* p)
{
  u8 ok = 0;                                                      //读取数据成功标志，0-不成功，1-成功

  if(s_structUARTSendBuf.dataLength > 0)                          //确保发送缓冲区有数据可读
  {
    ok = 1;                                                       //将标志设为1，表示读取数据成功
    *p = s_structUARTSendBuf.dataBuf[s_structUARTSendBuf.readPtr];//读取出发送缓冲区中的元素
    s_structUARTSendBuf.readPtr++;                                //读指针加1

    if(s_structUARTSendBuf.readPtr >= UART_BUF_SIZE)              //如果读指针大于或等于发送缓冲区的长度
    {
      s_structUARTSendBuf.readPtr = 0;                            //读指针清零
    }
    
    s_structUARTSendBuf.dataLength--;                             //读取出一个数据之后，发送缓冲区数据长度减1
  }
  
  return ok;                                                      //返回读取数据成功标志，0-不成功，1-成功 
}

/*********************************************************************************************************
* 函数名称: ConfigUART
* 函数功能: 配置UART的参数，包括GPIO、RCC、参数和NVIC  
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  void  ConfigUART(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;      //定义结构体GPIO_InitStructure，用来配置USART的GPIO
  USART_InitTypeDef USART_InitStructure;     //定义结构体USART_InitStructure，用来配置USART的常规参数
  NVIC_InitTypeDef  NVIC_InitStructure;      //定义结构体NVIC_InitStructure，用来配置USART的NVIC

  RCC_APB2PeriphClockCmd(USER_DEFINE_USART_TX_GPIO_CLK, ENABLE);     //开启USART的GPIO时钟
  if(USER_DEFINE_USART_TX_GPIO_CLK != USER_DEFINE_USART_RX_GPIO_CLK) //如果USART.TX和USART.RX的GPIO端口不同
  { 
    RCC_APB2PeriphClockCmd(USER_DEFINE_USART_RX_GPIO_CLK, ENABLE);   //开启USART.RX的GPIO时钟
  }
  
  if(USER_DEFINE_USART == USART1)                                    //如果串口为USART1
  { 
    RCC_APB2PeriphClockCmd(USER_DEFINE_USART_CLK, ENABLE);           //开启USART1的时钟
  }
  else
  {
    RCC_APB1PeriphClockCmd(USER_DEFINE_USART_CLK, ENABLE);           //开启USART2/3的时钟
  }
  
  //配置USART.Tx的GPIO
  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_USART_TX_GPIO_PIN;     //设置USART.TX的引脚
  GPIO_InitStructure.GPIO_Mode  = USER_DEFINE_USART_TX_MODE;         //设置USART.TX的模式
  GPIO_InitStructure.GPIO_Speed = USER_DEFINE_USART_TX_SPEED;        //设置USART.TX的I/O口速率
  GPIO_Init(USER_DEFINE_USART_TX_GPIO_PORT, &GPIO_InitStructure);    //根据参数初始化USART.TX的GPIO端口
  
  //配置USART.Rx的GPIO
  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_USART_RX_GPIO_PIN;     //设置USART.RX的引脚
  GPIO_InitStructure.GPIO_Mode  = USER_DEFINE_USART_RX_MODE;         //设置USART.RX的模式
  GPIO_InitStructure.GPIO_Speed = USER_DEFINE_USART_RX_SPEED;        //设置USART.RX的I/O口速率
  GPIO_Init(USER_DEFINE_USART_RX_GPIO_PORT, &GPIO_InitStructure);    //根据参数初始化USART.RX的GPIO端口
  
  //配置USART的常规参数
  USART_StructInit(&USART_InitStructure);                                //初始化串口的各项配置
  USART_InitStructure.USART_BaudRate   = USER_DEFINE_USART_BAUDRATE;     //设置USART的波特率
  USART_InitStructure.USART_WordLength = USER_DEFINE_USART_WORDLENGTH;   //设置USART的串口传输的字长
  USART_InitStructure.USART_StopBits   = USER_DEFINE_USART_STOPBITS;     //设置USART的停止位
  USART_InitStructure.USART_Parity     = USER_DEFINE_USART_PARITY;       //设置USART的奇偶校验位
  USART_InitStructure.USART_Mode       = USER_DEFINE_USART_MODE;         //设置USART的模式
  USART_InitStructure.USART_HardwareFlowControl = USER_DEFINE_USART_HRDWAREFLOWCONTROL;//设置USART的硬件流控制
  USART_Init(USER_DEFINE_USART, &USART_InitStructure);                   //根据参数初始化USART

  //配置USART的NVIC
  NVIC_InitStructure.NVIC_IRQChannel = USER_DEFINE_USART_IRQ;        //开启USART的中断
  //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;        //抢占优先级，屏蔽即默认值
  //NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;        //子优先级，屏蔽即默认值
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                    //IRQ通道使能
  NVIC_Init(&NVIC_InitStructure);                                    //根据参数初始化USART的NVIC

  //打开串口使能开关
  USART_ITConfig(USER_DEFINE_USART, USART_IT_RXNE, ENABLE);          //接收中断使能
  USART_ITConfig(USER_DEFINE_USART, USART_IT_TXE,  ENABLE);          //发送中断使能
  USART_Cmd(USER_DEFINE_USART, ENABLE);                              //使能USART
                                                                     
  s_iUARTTxSts = UART_STATE_OFF;                                     //初始化USART.TX状态标志
}

/*********************************************************************************************************
* 函数名称: EnableUARTTx
* 函数功能: 使能串口发送，在WriteUart中调用，即每次发送数据之后需要调用这个函数来打开TXE的中断开关 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: s_iUARTTxSts = UART_STATE_ON;这句话必须放在USART_ITConfig之前，否则会导致中断打开无法执行
*********************************************************************************************************/
static  void  EnableUARTTx(void)
{
  s_iUARTTxSts = UART_STATE_ON;                               //将USART.TX的状态标志设置为打开

  USART_ITConfig(USER_DEFINE_USART, USART_IT_TXE, ENABLE);    //发送中断使能
}

/*********************************************************************************************************
* 函数名称: SendCharUsedByFputc
* 函数功能: 发送字符函数，专由fputc函数调用  
* 输入参数: ch，待发送的字符
* 输出参数: void
* 返 回 值: void 
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  void  SendCharUsedByFputc(u16 ch)
{
  USART_SendData(USER_DEFINE_USART, (u8)ch);//串口发送ch           

  //当串口发送完成，跳出循环
  while(USART_GetFlagStatus(USER_DEFINE_USART, USART_FLAG_TC) == RESET)  
  {
    
  }
}

/*********************************************************************************************************
* 函数名称: USER_DEFINE_USART_IRQHandler
* 函数功能: USART中断服务函数，由用户自定义
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void USER_DEFINE_USART_IRQHandler(void)                
{
  u8  uData = 0;

  if(USART_GetITStatus(USER_DEFINE_USART, USART_IT_RXNE) != RESET)   //接收中断
  {
    NVIC_ClearPendingIRQ(USER_DEFINE_USART_IRQ);                     //清除接收中断
    uData = USART_ReceiveData(USER_DEFINE_USART);                    //接收串口传来的数据，并存到uData中
    //当接收到数据时，将接收到的数据写入到接收缓冲区中 
    WriteReceiveBuf(uData);
  }
  
  if(USART_GetFlagStatus(USER_DEFINE_USART, USART_FLAG_ORE) == SET) //如果USART溢出错误标志位为“1”
  {
    USART_ClearFlag(USER_DEFINE_USART, USART_FLAG_ORE);             //清除USART的溢出错误标志
    USART_ReceiveData(USER_DEFINE_USART);                           //从USART接收数据 
  }
  
  if(USART_GetITStatus(USER_DEFINE_USART, USART_IT_TXE) != RESET)   //发送中断
  {
    USART_ClearITPendingBit(USER_DEFINE_USART, USART_IT_TXE);       //清除USART的发送中断标志位
    NVIC_ClearPendingIRQ(USER_DEFINE_USART_IRQ);                    //清除发送中断
    
    ReadSendBuf(&uData);                                            //读取发送缓冲区的数据到uData
    
    USART_SendData(USER_DEFINE_USART, uData);                       //串口发送数据uData
    
    if(0 == s_structUARTSendBuf.dataLength)                         //当发送缓冲区数据长度为0
    {
      s_iUARTTxSts = UART_STATE_OFF;                                //USART.TX状态标志设置为关闭                      
      USART_ITConfig(USER_DEFINE_USART, USART_IT_TXE, DISABLE);     //关闭串口发送中断
    }
  }
}  

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitUART
* 函数功能: 初始化USART 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void InitUART(void)
{
  InitUARTBuf();  //初始化串口缓冲区，包括发送缓冲区和接收缓冲区  

  ConfigUART();   //配置UART的参数，包括GPIO、RCC、常规参数和NVIC
}

/*********************************************************************************************************
* 函数名称: WriteUART
* 函数功能: 写数据到发送缓冲区
* 输入参数: pBuf，要写入数据的首地址，len，欲写入数据的长度
* 输出参数: void
* 返 回 值: 实际写入数据的长度，不一定与输入参数len相等
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8  WriteUART(u8* pBuf, u8 len)
{
  u8 i;                                                               //i为循环计数器
  u8 wLen = 0;                                                        //实际写入数据的长度
                                                                      
  if(s_structUARTSendBuf.dataLength < UART_BUF_SIZE)                  //当发送缓冲区数据数小于缓冲区大小
  {                                                                                                                                      
    wLen = len;                                                       //实际写入数据长度等于欲写入数据长度
                                                                      
    //如果当前发送缓冲区数据长度加上欲写入数据长度大于发送缓冲区大小  
    if((s_structUARTSendBuf.dataLength + len) > UART_BUF_SIZE)         
    {                                                                 
      wLen = UART_BUF_SIZE - s_structUARTSendBuf.dataLength;          //实际写入数据长度等于发送缓冲区剩余大小
    }                                                                 
                                                                      
    for(i = 0; i < wLen; i++)                                         //循环直至数据全部写入发送缓冲区
    {
      s_structUARTSendBuf.dataBuf[s_structUARTSendBuf.writePtr] = pBuf[i];//向发送缓冲区写入数据的第i位
      s_structUARTSendBuf.writePtr++;                                     //写指针加1
    
      if(s_structUARTSendBuf.writePtr >= UART_BUF_SIZE)                   //如果写指针超出缓冲区大小
      {  
        s_structUARTSendBuf.writePtr = 0;                                 //重置写指针为0
      }

      s_structUARTSendBuf.dataLength++;   //发送缓冲区数据长度加1
    }

    if(s_iUARTTxSts == UART_STATE_OFF)    //如果串口TX的状态为关闭
    {
      EnableUARTTx();                     //使能串口发送，打开TXE的中断开关
    }
  }
  
  return wLen;                            //返回实际写入数据的长度
}

/*********************************************************************************************************
* 函数名称: ReadUART
* 函数功能: 读取相应接收缓冲区中的数据  
* 输入参数: len，欲读取数据的长度
* 输出参数: pBuf，读取的数据存放的首地址
* 返 回 值: 实际读取数据的长度
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8  ReadUART(u8* pBuf, u8 len)
{
  u8 i;                                                 //i为循环计数器
  u8 rLen = 0;                                          //实际读取数据长度
  
  if(s_structUARTRecBuf.dataLength > 0)                 //接收缓冲区中有数据
  {
    rLen = len;                                         //实际读取数据长度等于欲读取数据长度

    if(s_structUARTRecBuf.dataLength < len)             //欲读取数据的长度超过接收缓冲区的数据长度
    {     
      rLen = s_structUARTRecBuf.dataLength;             //实际读取数据长度等于接收缓冲区的数据长度
    }

    for(i = 0; i < rLen; i++)                           //循环读取接收缓冲区中的数据
    {
      pBuf[i] = s_structUARTRecBuf.dataBuf[s_structUARTRecBuf.readPtr];//读取接收缓冲区的数据并存到pBuf第i位
      s_structUARTRecBuf.readPtr++;                                    //读指针加1

      if(s_structUARTRecBuf.readPtr >= UART_BUF_SIZE)                  //如果读指针超出接收缓冲区的大小
      {  
        s_structUARTRecBuf.readPtr = 0;                                //重置读指针为0
      }
      s_structUARTRecBuf.dataLength--;                                 //接收缓冲区数据长度减1
    }
  }

  return rLen;                                          //返回实际读取数据的长度
}

/*********************************************************************************************************
* 函数名称: fputc
* 函数功能: 重定向函数  
* 输入参数: ch，f
* 输出参数: void
* 返 回 值: int 
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
int fputc(int ch, FILE* f)
{
  SendCharUsedByFputc((u8) ch);   //发送字符函数，专由fputc函数调用
  
  return ch;                      //返回ch
}

/*********************************************************************************************************
* 函数名称: fgetc
* 函数功能: 重定向函数  
* 输入参数: f
* 输出参数: void
* 返 回 值: 串口接收到的数据 
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
int fgetc(FILE* f)
{
  //当串口接收不为空时，跳出循环
  while(USART_GetFlagStatus(USER_DEFINE_USART, USART_FLAG_RXNE == RESET))
  {
    
  }
  
  return(USART_ReceiveData(USER_DEFINE_USART)); //返回串口接收到的数据
}
