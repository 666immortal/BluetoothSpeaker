/*********************************************************************************************************
* 模块名称: UART2.c
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
#include "UART2.h"
#include <string.h>
#include "ExeCode.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              结构体枚举定义
*********************************************************************************************************/
//工作模式 
typedef enum
{
  UART2_STATE_OFF,         //USART.TX状态为关闭
	UART2_STATE_ON,          //USART.TX状态为打开
	UART2_STATE_MAX          
}EnumUART2State;           

//串口缓冲区定义
typedef struct
{
  u8  dataBuf2[UART2_BUF_SIZE];   //数据缓冲区
  u8  readPtr2;                   //读指针
  u8  writePtr2;                  //写指针
  u8  dataLength2;                //数据长度
}StructUart2Buf;                 

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static  StructUart2Buf s_structUART2SendBuf;  //发送串口buf
static  StructUart2Buf s_structUART2RecBuf;   //接收串口buf
static  u8 s_iUART2TxSts;                    //USART.TX状态标志

extern u8 initBTFlag;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static  void  InitUART2Buf(void);            //初始化串口缓冲区，包括发送缓冲区和接收缓冲区 
static  u8    WriteReceiveBuf2(u8 d);        //将接收到的数据写入到接收缓冲区中
static  u8    ReadSendBuf2(u8* p);           //读取发送缓冲区中的数据

static  void  ConfigUART2(void);             //配置UART的参数，包括GPIO、RCC、USART常规参数和NVIC 
static  void  EnableUARTT2(void);           //使能串口发送，在WriteUart中调用，即每次发送数据之后需要调用
                                            //打开TXE的中断开关

  
/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitUART2Buf
* 函数功能: 初始化串口缓冲区，包括发送缓冲区和接收缓冲区  
* 输入参数: void
* 输出参数: void
* 返 回 值: void 
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  void  InitUART2Buf(void)
{
  s_structUART2SendBuf.dataLength2 = 0;  //初始化串口发送缓冲区数据长度
  s_structUART2SendBuf.readPtr2    = 0;  //初始化读指针
  s_structUART2SendBuf.writePtr2   = 0;  //初始化写指针
  
  s_structUART2RecBuf.dataLength2  = 0;  //初始化串口接收缓冲区数据长度
  s_structUART2RecBuf.readPtr2     = 0;  //初始化读指针
  s_structUART2RecBuf.writePtr2    = 0;  //初始化写指针
}

/*********************************************************************************************************
* 函数名称: WriteReceiveBuf2
* 函数功能: 写数据到串口接收BUF，在中断服务函数中，当接收到数据时，将接收到的数据写入到串口接收BUF中 
* 输入参数: d，串口中断服务函数接收到的数据
* 输出参数: void
* 返 回 值: 写入数据成功标志，0-不成功，1-成功 
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  u8  WriteReceiveBuf2(u8 d)
{
  u8 ok = 0;                                                      //写入数据成功标志，0-不成功，1-成功

  if(s_structUART2RecBuf.dataLength2 < UART2_BUF_SIZE)               //确保接收缓冲区有空间可写
  {
    ok = 1;                                                       //将标志设为1，表示写入数据成功
    s_structUART2RecBuf.dataBuf2[s_structUART2RecBuf.writePtr2] = d;  //写数据到接收缓冲区
    s_structUART2RecBuf.writePtr2++;                                //写指针加1        

    if(s_structUART2RecBuf.writePtr2 >= UART2_BUF_SIZE)              //如果写指针大于或等于接收缓冲区的长度
    {
      s_structUART2RecBuf.writePtr2 = 0;                            //写指针清零
    }
    
    s_structUART2RecBuf.dataLength2++;                              //写入一个数据之后，接收缓冲区数据长度加1
  }
  
  return ok;                                                      //返回写入数据成功标志，0-不成功，1-成功 
}

/*********************************************************************************************************
* 函数名称: ReadSendBuf2
* 函数功能: 读取发送BUF，用户通过WriteUart将要写入的数据传至发送缓冲，中断函数通过接收缓冲读出 
* 输入参数: void
* 输出参数: p，读出来的数据存放的首地址
* 返 回 值: 读取数据成功标志，0-不成功，1-成功 
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  u8  ReadSendBuf2(u8* p)
{
  u8 ok = 0;                                                      //读取数据成功标志，0-不成功，1-成功

  if(s_structUART2SendBuf.dataLength2 > 0)                          //确保发送缓冲区有数据可读
  {
    ok = 1;                                                       //将标志设为1，表示读取数据成功
    *p = s_structUART2SendBuf.dataBuf2[s_structUART2SendBuf.readPtr2];//读取出发送缓冲区中的元素
    s_structUART2SendBuf.readPtr2++;                                //读指针加1

    if(s_structUART2SendBuf.readPtr2 >= UART2_BUF_SIZE)              //如果读指针大于或等于发送缓冲区的长度
    {
      s_structUART2SendBuf.readPtr2 = 0;                            //读指针清零
    }
    
    s_structUART2SendBuf.dataLength2--;                             //读取出一个数据之后，发送缓冲区数据长度减1
  }
  
  return ok;                                                      //返回读取数据成功标志，0-不成功，1-成功 
}

/*********************************************************************************************************
* 函数名称: ConfigUART2
* 函数功能: 配置UART的参数，包括GPIO、RCC、参数和NVIC  
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  void  ConfigUART2(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;      //定义结构体GPIO_InitStructure，用来配置USART的GPIO
  USART_InitTypeDef USART_InitStructure;     //定义结构体USART_InitStructure，用来配置USART的常规参数
  NVIC_InitTypeDef  NVIC_InitStructure;      //定义结构体NVIC_InitStructure，用来配置USART的NVIC

  RCC_APB2PeriphClockCmd(USER_DEFINE_USART2_TX_GPIO_CLK, ENABLE);     //开启USART的GPIO时钟
  if(USER_DEFINE_USART2_TX_GPIO_CLK != USER_DEFINE_USART2_RX_GPIO_CLK) //如果USART.TX和USART.RX的GPIO端口不同
  { 
    RCC_APB2PeriphClockCmd(USER_DEFINE_USART2_RX_GPIO_CLK, ENABLE);   //开启USART.RX的GPIO时钟
  }
  
  if(USER_DEFINE_USART2 == USART1)                                    //如果串口为USART1
  { 
    RCC_APB2PeriphClockCmd(USER_DEFINE_USART2_CLK, ENABLE);           //开启USART1的时钟
  }
  else
  {
    RCC_APB1PeriphClockCmd(USER_DEFINE_USART2_CLK, ENABLE);           //开启USART2/3的时钟
  }
  
  //配置USART.Tx的GPIO
  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_USART2_TX_GPIO_PIN;     //设置USART.TX的引脚
  GPIO_InitStructure.GPIO_Mode  = USER_DEFINE_USART2_TX_MODE;         //设置USART.TX的模式
  GPIO_InitStructure.GPIO_Speed = USER_DEFINE_USART2_TX_SPEED;        //设置USART.TX的I/O口速率
  GPIO_Init(USER_DEFINE_USART2_TX_GPIO_PORT, &GPIO_InitStructure);    //根据参数初始化USART.TX的GPIO端口
  
  //配置USART.Rx的GPIO
  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_USART2_RX_GPIO_PIN;     //设置USART.RX的引脚
  GPIO_InitStructure.GPIO_Mode  = USER_DEFINE_USART2_RX_MODE;         //设置USART.RX的模式
  GPIO_InitStructure.GPIO_Speed = USER_DEFINE_USART2_RX_SPEED;        //设置USART.RX的I/O口速率
  GPIO_Init(USER_DEFINE_USART2_RX_GPIO_PORT, &GPIO_InitStructure);    //根据参数初始化USART.RX的GPIO端口
  
  //配置USART的常规参数
  USART_StructInit(&USART_InitStructure);                                //初始化串口的各项配置
  USART_InitStructure.USART_BaudRate   = USER_DEFINE_USART2_BAUDRATE;     //设置USART的波特率
  USART_InitStructure.USART_WordLength = USER_DEFINE_USART2_WORDLENGTH;   //设置USART的串口传输的字长
  USART_InitStructure.USART_StopBits   = USER_DEFINE_USART2_STOPBITS;     //设置USART的停止位
  USART_InitStructure.USART_Parity     = USER_DEFINE_USART2_PARITY;       //设置USART的奇偶校验位
  USART_InitStructure.USART_Mode       = USER_DEFINE_USART2_MODE;         //设置USART的模式
  USART_InitStructure.USART_HardwareFlowControl = USER_DEFINE_USART2_HRDWAREFLOWCONTROL;//设置USART的硬件流控制
  USART_Init(USER_DEFINE_USART2, &USART_InitStructure);                   //根据参数初始化USART

  //配置USART的NVIC
  NVIC_InitStructure.NVIC_IRQChannel = USER_DEFINE_USART2_IRQ;        //开启USART的中断
  //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;        //抢占优先级，屏蔽即默认值
  //NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;        //子优先级，屏蔽即默认值
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                    //IRQ通道使能
  NVIC_Init(&NVIC_InitStructure);                                    //根据参数初始化USART的NVIC

  //打开串口使能开关
  USART_ITConfig(USER_DEFINE_USART2, USART_IT_RXNE, ENABLE);          //接收中断使能
  USART_ITConfig(USER_DEFINE_USART2, USART_IT_TXE,  ENABLE);          //发送中断使能
  USART_Cmd(USER_DEFINE_USART2, ENABLE);                              //使能USART
                                                                     
  s_iUART2TxSts = UART2_STATE_OFF;                                     //初始化USART.TX状态标志
}

/*********************************************************************************************************
* 函数名称: EnableUARTT2
* 函数功能: 使能串口发送，在WriteUart中调用，即每次发送数据之后需要调用这个函数来打开TXE的中断开关 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: s_iUART2TxSts = UART2_STATE_ON;这句话必须放在USART_ITConfig之前，否则会导致中断打开无法执行
*********************************************************************************************************/
static  void  EnableUARTT2(void)
{
  s_iUART2TxSts = UART2_STATE_ON;                               //将USART.TX的状态标志设置为打开

  USART_ITConfig(USER_DEFINE_USART2, USART_IT_TXE, ENABLE);    //发送中断使能
}

/*********************************************************************************************************
* 函数名称: USER_DEFINE_USART2_IRQHandler
* 函数功能: USART中断服务函数，由用户自定义
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void USER_DEFINE_USART2_IRQHandler(void)                
{
  u8  uData = 0;

  if(USART_GetITStatus(USER_DEFINE_USART2, USART_IT_RXNE) != RESET)   //接收中断
  {
    NVIC_ClearPendingIRQ(USER_DEFINE_USART2_IRQ);                     //清除接收中断
    uData = USART_ReceiveData(USER_DEFINE_USART2);                    //接收串口传来的数据，并存到uData中
    //当接收到数据时，将接收到的数据写入到接收缓冲区中 
    WriteReceiveBuf2(uData);
  }
  
  if(USART_GetFlagStatus(USER_DEFINE_USART2, USART_FLAG_ORE) == SET) //如果USART溢出错误标志位为“1”
  {
    USART_ClearFlag(USER_DEFINE_USART2, USART_FLAG_ORE);             //清除USART的溢出错误标志
    USART_ReceiveData(USER_DEFINE_USART2);                           //从USART接收数据 
  }
  
  if(USART_GetITStatus(USER_DEFINE_USART2, USART_IT_TXE) != RESET)   //发送中断
  {
    USART_ClearITPendingBit(USER_DEFINE_USART2, USART_IT_TXE);       //清除USART的发送中断标志位
    NVIC_ClearPendingIRQ(USER_DEFINE_USART2_IRQ);                    //清除发送中断
    
    ReadSendBuf2(&uData);                                            //读取发送缓冲区的数据到uData
    
    USART_SendData(USER_DEFINE_USART2, uData);                       //串口发送数据uData
    
    if(0 == s_structUART2SendBuf.dataLength2)                         //当发送缓冲区数据长度为0
    {
      s_iUART2TxSts = UART2_STATE_OFF;                                //USART.TX状态标志设置为关闭                      
      USART_ITConfig(USER_DEFINE_USART2, USART_IT_TXE, DISABLE);     //关闭串口发送中断
    }
  }
}  

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitUART2
* 函数功能: 初始化USART 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void InitUART2(void)
{
  InitUART2Buf();  //初始化串口缓冲区，包括发送缓冲区和接收缓冲区  

  ConfigUART2();   //配置UART的参数，包括GPIO、RCC、常规参数和NVIC
}

/*********************************************************************************************************
* 函数名称: WriteUART2
* 函数功能: 写数据到发送缓冲区
* 输入参数: pBuf，要写入数据的首地址，len，欲写入数据的长度
* 输出参数: void
* 返 回 值: 实际写入数据的长度，不一定与输入参数len相等
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8  WriteUART2(u8* pBuf, u8 len)
{
  u8 i;                                                               //i为循环计数器
  u8 wLen = 0;                                                        //实际写入数据的长度
                                                                      
  if(s_structUART2SendBuf.dataLength2 < UART2_BUF_SIZE)                  //当发送缓冲区数据数小于缓冲区大小
  {                                                                                                                                      
    wLen = len;                                                       //实际写入数据长度等于欲写入数据长度
                                                                      
    //如果当前发送缓冲区数据长度加上欲写入数据长度大于发送缓冲区大小  
    if((s_structUART2SendBuf.dataLength2 + len) > UART2_BUF_SIZE)         
    {                                                                 
      wLen = UART2_BUF_SIZE - s_structUART2SendBuf.dataLength2;          //实际写入数据长度等于发送缓冲区剩余大小
    }                                                                 
                                                                      
    for(i = 0; i < wLen; i++)                                         //循环直至数据全部写入发送缓冲区
    {
      s_structUART2SendBuf.dataBuf2[s_structUART2SendBuf.writePtr2] = pBuf[i];//向发送缓冲区写入数据的第i位
      s_structUART2SendBuf.writePtr2++;                                     //写指针加1
    
      if(s_structUART2SendBuf.writePtr2 >= UART2_BUF_SIZE)                   //如果写指针超出缓冲区大小
      {  
        s_structUART2SendBuf.writePtr2 = 0;                                 //重置写指针为0
      }

      s_structUART2SendBuf.dataLength2++;   //发送缓冲区数据长度加1
    }

    if(s_iUART2TxSts == UART2_STATE_OFF)    //如果串口TX的状态为关闭
    {
      EnableUARTT2();                       //使能串口发送，打开TXE的中断开关
    }
  }
  
  return wLen;                              //返回实际写入数据的长度
}

/*********************************************************************************************************
* 函数名称: ReadUART2
* 函数功能: 读取相应接收缓冲区中的数据  
* 输入参数: len，欲读取数据的长度
* 输出参数: pBuf，读取的数据存放的首地址
* 返 回 值: 实际读取数据的长度
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8  ReadUART2(u8* pBuf, u8 len)
{
  u8 i;                                                   //i为循环计数器
  u8 rLen = 0;                                            //实际读取数据长度
  
  if(s_structUART2RecBuf.dataLength2 > 0)                 //接收缓冲区中有数据
  {
    rLen = len;                                           //实际读取数据长度等于欲读取数据长度

    if(s_structUART2RecBuf.dataLength2 < len)             //欲读取数据的长度超过接收缓冲区的数据长度
    {     
      rLen = s_structUART2RecBuf. dataLength2;            //实际读取数据长度等于接收缓冲区的数据长度
    }

    for(i = 0; i < rLen; i++)                             //循环读取接收缓冲区中的数据
    {
      pBuf[i] = s_structUART2RecBuf.dataBuf2[s_structUART2RecBuf.readPtr2]; //读取接收缓冲区的数据并存到pBuf第i位
      s_structUART2RecBuf.readPtr2++;                                       //读指针加1

      if(s_structUART2RecBuf.readPtr2 >= UART2_BUF_SIZE)                  //如果读指针超出接收缓冲区的大小
      {  
        s_structUART2RecBuf.readPtr2 = 0;                                //重置读指针为0
      }
      s_structUART2RecBuf.dataLength2--;                                 //接收缓冲区数据长度减1
    }
  }

  return rLen;                                          //返回实际读取数据的长度
}

/*********************************************************************************************************
* 函数名称: WaitForReceive
* 函数功能: 等待蓝牙发送应答
* 输入参数: void
* 输出参数: void
* 返 回 值: 1：接收到OK
* 创建日期: 2018年03月01日
* 注    意: 记得清楚Timer里面的串口接收和发送
*********************************************************************************************************/
u8 WaitForReceive(const u8* cmd)
{
//  const u8 cmd[] = "OK";
  u8 recData;
  u8 getData[15];
  u8 i = 0;
  u8 dataFlag = 0;
  
  while(ReadUART2(&recData, 1))                //如果串口接收到了数据
  {   
    if(recData != 0x0D && recData != 0X0A)    //接收到的数据不是0X0A或者0X0D
    {   
      getData[i] = recData;                   //将数据取出
      i++;      
    }   
    else    
    {   
      if(recData == 0X0A)                     //如果接受到了0X0A，在字符串末尾加上'\0'
      {   
        getData[i] = '\0';                    
        dataFlag = 1;      //打印接受到了指令
        printf("I receive %s\r\n", getData);
        break;    
      }   
      else if (recData == 0X0D)               //如果接收到了0X0D，那么跳过操作
      {
        
      }
    }
  }
  
  if(dataFlag == 1)
  {
    if(strcmp((char*)getData, (char*)cmd) == 0) //比较接收到的字符串和指令是否一致
    {
      return 1;
    }
  }
  
  return 0;
}

/*********************************************************************************************************
 * 函数名称: WaitForTask
* 函数功能: 等待蓝牙发送任务
* 输入参数: receiveTask：接收到的任务代号
* 输出参数: void
* 返 回 值: 1：接收到任务
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8 WaitForTask(u8* receiveTask)
{
  u8 recData;
  u8 getData[15];
  u8 i = 0;
  u8 dataFlag = 0;
  
  while(ReadUART2(&recData, 1))                //如果串口接收到了数据
  {   
    if(recData != 0x0D && recData != 0X0A)    //接收到的数据不是0X0A或者0X0D
    {   
      getData[i] = recData;                   //将数据取出
      i++;      
    }   
    else    
    {   
      if(recData == 0X0A)                     //如果接受到了0X0A，在字符串末尾加上'\0'
      {   
        getData[i] = '\0';                    
        dataFlag = 1;      //打印接受到了指令
        printf("NOW I receive %s\r\n", getData);
        break;    
      }   
      else if (recData == 0X0D)               //如果接收到了0X0D，那么跳过操作
      {
        
      }
    }
  }
  
  if(dataFlag == 1)
  {
    if(getData[0] == (u8)'A' && getData[1] == (u8)'P' && getData[2] == (u8)'R' && getData[3] == (u8)'+')
    {
      receiveTask = getData + 4;
      printf("I receive code:%s\r\n", receiveTask);
      ExeBTCode(receiveTask);
      return 1;
    }
    
    if(getData[0] == 'I' && getData[1] == 'A')
    {
      initBTFlag = 0;
    }
  }
  
  return 0;
}

///*********************************************************************************************************
//* 函数名称: ReadAndShow
//* 函数功能: 读串口然后显示
//* 输入参数: void
//* 输出参数: void
//* 返 回 值: void
//* 创建日期: 2018年03月01日
//* 注    意: 
//*********************************************************************************************************/
//void ReadAndShow(void)
//{
//  u8 recData;
//  u8 getData[10];
//  u8 i = 0;
//  
//  while(ReadUART2(&recData, 1))                //如果串口接收到了数据
//  {   
//    if(recData != 0x0D && recData != 0X0A)    //接收到的数据不是0X0A或者0X0D
//    {   
//      getData[i] = recData;                   //将数据取出
//      i++;      
//    }   
//    else    
//    {   
//      if(recData == 0X0A)                     //如果接受到了0X0A，在字符串末尾加上'\0'
//      {   
//        getData[i] = '\0';                    
//        printf("%s", getData);
//        break;    
//      }   
//      else if (recData == 0X0D)               //如果接收到了0X0D，那么跳过操作
//      {
//        
//      }
//    }
//  }
//}
