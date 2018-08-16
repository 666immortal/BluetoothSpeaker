/*********************************************************************************************************
* 模块名称: W25Qxx.c
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
#include "W25Qxx.h" 
#include "SPI2.h"
#include "SysTick.h"   

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
//W25Qxx指令表
#define W25X_WRITE_ENABLE            0x06     //写使能
#define W25X_WRITE_DISABLE           0x04     //写禁用
#define W25X_READ_STS_REG            0x05     //读状态寄存器
#define W25X_WRITE_STS_REG           0x01     //写状态寄存器
#define W25X_READ_DATA               0x03     //读数据
#define W25X_FAST_READ_DATA          0x0B     //快速读数据
#define W25X_FAST_READ_DUAL          0x3B     //双倍速度读数据
#define W25X_PAGE_PROGRAM            0x02     //写页命令
#define W25X_BLOCK_ERASE             0xD8     //擦除块
#define W25X_SECTOR_ERASE            0x20     //擦除扇区
#define W25X_CHIP_ERASE              0xC7     //擦除芯片
#define W25X_POWER_DOWN              0xB9     //掉电模式
#define W25X_RELEASE_POWER_DOWN      0xAB     //释放掉电模式
#define W25X_DEVICE_ID               0xAB     //设备ID
#define W25X_MANUFACT_DEVICE_ID      0x90     //制造商ID
#define W25X_JEDEC_DEVICE_ID         0x9F     //JEDEC标准ID

//选中W25Qxx的片选
#define CLR_W25Qxx_CS()	GPIO_ResetBits(USER_DEFINE_SPI2_CS_GPIO_PORT, USER_DEFINE_SPI2_CS_GPIO_PIN)
//取消W25Qxx的片选
#define SET_W25Qxx_CS()	GPIO_SetBits  (USER_DEFINE_SPI2_CS_GPIO_PORT, USER_DEFINE_SPI2_CS_GPIO_PIN)

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static u8 SPI_FLASH_BUF[4096];                //SPI缓冲区

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static  u8   W25QxxReadSR(void);              //读状态寄存器 
//static  void W25QxxWriteSR(u8 sr);            //写状态寄存器
static  void W25QxxWriteEnable(void);         //写使能 
//static  void W25QxxWriteDisable(void);        //写保护
static  void W25QxxWaitBusy(void);            //等待空闲
//在指定地址开始写入最大256字节的数据
static  void W25QxxWritePage(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
//无检验写W25Qxx,在指定地址开始写入指定长度的数据
static  void W25QxxWriteNoCheck(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
//static  void W25QxxPowerDown(void);           //进入掉电模式

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: W25QxxReadSR
* 函数功能: 读取W25Qxx的状态寄存器 
* 输入参数: void
* 输出参数: void
* 返 回 值: 状态寄存器
* 创建日期: 2018年03月01日
* 注    意: BIT7  6   5   4   3   2   1   0
*           SPR   RV  TB BP2 BP1 BP0 WEL BUSY
*           SPR:默认0,状态寄存器保护位,配合WP使用
*           TB,BP2,BP1,BP0:FLASH区域写保护设置
*           WEL:写使能锁定
*           BUSY:忙标记位(1,忙;0,空闲)
*           默认:0x00
*********************************************************************************************************/
static  u8 W25QxxReadSR(void)   
{  
  u8 byte=0;                                //用来存放寄存器的状态
  
  CLR_W25Qxx_CS();                          //选中W25Qxx的片选
  SPI2ReadWriteByte(W25X_READ_STS_REG);      //发送读取状态寄存器命令    
  byte = SPI2ReadWriteByte(0Xff);            //读取一个字节
  SET_W25Qxx_CS();                          //取消W25Qxx的片选

  return byte;                              //返回寄存器的状态
} 

///*********************************************************************************************************
//* 函数名称: W25QxxWriteSR
//* 函数功能: 写W25Qxx状态寄存器 
//* 输入参数: sr，待写入的状态寄存器
//* 输出参数: void
//* 返 回 值: void
//* 创建日期: 2018年03月01日
//* 注    意: 只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写
//*********************************************************************************************************/
//static  void W25QxxWriteSR(u8 sr)   
//{   
//  CLR_W25Qxx_CS();                          //选中W25Qxx的片选
//  SPI2ReadWriteByte(W25X_WRITE_STS_REG);     //发送写取状态寄存器命令    
//  SPI2ReadWriteByte(sr);                     //写入一个字节  
//  SET_W25Qxx_CS();                          //取消W25Qxx的片选
//} 
//  
/*********************************************************************************************************
* 函数名称: W25QxxWriteEnable
* 函数功能: 写使能，将WEL置位 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意:   
*********************************************************************************************************/ 
static  void W25QxxWriteEnable(void)   
{
  CLR_W25Qxx_CS();                            //选中W25Qxx的片选
  SPI2ReadWriteByte(W25X_WRITE_ENABLE);        //发送写使能指令
  SET_W25Qxx_CS();                            //取消W25Qxx的片选
} 

///*********************************************************************************************************
//* 函数名称: W25QxxWriteDisable
//* 函数功能: 写禁止，将WEL清零 
//* 输入参数: void
//* 输出参数: void
//* 返 回 值: void
//* 创建日期: 2018年03月01日
//* 注    意: 
//*********************************************************************************************************/ 
//static  void W25QxxWriteDisable(void)   
//{  
//  CLR_W25Qxx_CS();                            //选中W25Qxx的片选
//  SPI2ReadWriteByte(W25X_WRITE_DISABLE);       //发送写禁止指令    
//  SET_W25Qxx_CS();                            //取消W25Qxx的片选
//}
// 
/*********************************************************************************************************
* 函数名称: W25QxxWritePage
* 函数功能: 在一页(0~65535)内写入少于256个字节的数据，在指定地址开始写入最大256字节的数据 
* 输入参数: pBuffer:数据存储区，WriteAddr:开始写入的地址(24bit)
*           NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  void W25QxxWritePage(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  u16 i;                                      //循环计数器
  W25QxxWriteEnable();                        //写使能，即将WEL置位 
  CLR_W25Qxx_CS();                            //选中W25Qxx的片选

  SPI2ReadWriteByte(W25X_PAGE_PROGRAM);        //发送写页命令   
  SPI2ReadWriteByte((u8)((WriteAddr) >> 16));  //发送24bit地址    
  SPI2ReadWriteByte((u8)((WriteAddr) >> 8));   
  SPI2ReadWriteByte((u8)WriteAddr);   
  for(i = 0; i < NumByteToWrite; i++)
  {
    SPI2ReadWriteByte(pBuffer[i]);             //循环写数
  }
  
  SET_W25Qxx_CS();                            //取消W25Qxx的片选

  W25QxxWaitBusy();                           //等待写入结束
} 

/*********************************************************************************************************
* 函数名称: W25QXXWriteNoCheck
* 函数功能: 无检验写W25Qxx,在指定地址开始写入指定长度的数据
* 输入参数: pBuffer:数据存储区,WriteAddr:开始写入的地址(24bit),NumByteToWrite:要写入的字节数(最大65535)
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
*           具有自动换页功能 
*           在指定地址开始写入指定长度的数据,但是要确保地址不越界!
*********************************************************************************************************/
static  void W25QxxWriteNoCheck(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)   
{
  u16 pageRemain;                       //用于存放单页剩余的字节数
  pageRemain = 256 - WriteAddr % 256;   //单页剩余的字节数
  if(NumByteToWrite <= pageRemain)
  {
    pageRemain = NumByteToWrite;        //要写入的字节数不大于256个字节
  }
  
  while(1)
  { 
    //在指定地址开始写入数据  
    W25QxxWritePage(pBuffer, WriteAddr, pageRemain);
    
    if(NumByteToWrite == pageRemain)
    {
      break;                            //写入结束
    }
    else
    {
      pBuffer   += pageRemain;
      WriteAddr += pageRemain;

      NumByteToWrite -= pageRemain;     //减去已经写入了的字节数
      
      if(NumByteToWrite > 256)
      {
        pageRemain = 256;               //一次可以写入256个字节
      }
      else
      {
        pageRemain = NumByteToWrite;    //不够256个字节了
      }
    }
  }
} 

/*********************************************************************************************************
* 函数名称: W25QXXWaitBusy
* 函数功能: 等待空闲 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static  void W25QxxWaitBusy(void)   
{   
  while((W25QxxReadSR() & 0x01) == 0x01)     //等待BUSY位清空
  {
    
  }
}
  
///*********************************************************************************************************
//* 函数名称: W25QXXPowerDown
//* 函数功能: 进入掉电模式 
//* 输入参数: void
//* 输出参数: void
//* 返 回 值: void
//* 创建日期: 2018年03月01日
//* 注    意: 
//*********************************************************************************************************/
//static  void W25QxxPowerDown(void)   
//{ 
//  CLR_W25Qxx_CS();                            //选中W25Qxx的片选
//  SPI2ReadWriteByte(W25X_POWER_DOWN);          //发送掉电模式命令  
//  SET_W25Qxx_CS();                            //取消W25Qxx的片选
//  DelayNus(3);                                //延时3us，等待TPD  
//}  

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitW25Qxx
* 函数功能: 初始化W25QXX 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 4Kbytes为一个Sector           
*           16个扇区为1个Block
*           W25X16
*           容量为2M字节,共有32个Block,512个Sector 
*********************************************************************************************************/
void InitW25Qxx(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;//定义结构体GPIO_InitStructure，用来配置W25Qxx片选的GPIO
  
  RCC_APB2PeriphClockCmd(USER_DEFINE_SPI2_CS_GPIO_CLK, ENABLE);   //使能W25Qxx片选的时钟

  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_SPI2_CS_GPIO_PIN;   //设置25Qxx片选的引脚    
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;              //推挽输出模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;              //设置I/O口速率为50MHz
  GPIO_Init(USER_DEFINE_SPI2_CS_GPIO_PORT, &GPIO_InitStructure);  //根据参数初始化25Qxx片选的GPIO端口

  SET_W25Qxx_CS();                                               //取消W25Qxx的片选
  InitSPI2();                                                     //初始化SPI
  SPI2SetSpeed(SPI_BaudRatePrescaler_2);                          //设置为18M时钟,高速模式
}

/*********************************************************************************************************
* 函数名称: W25QxxReadID
* 函数功能: 读取芯片ID
* 输入参数: void
* 输出参数: void
* 返 回 值: 读取到的芯片的ID
* 创建日期: 2018年03月01日
* 注    意: 0XEF13,表示芯片型号为W25Q80  
*           0XEF14,表示芯片型号为W25Q16    
*           0XEF15,表示芯片型号为W25Q32  
*           0XEF16,表示芯片型号为W25Q64 
*           0XEF17,表示芯片型号为W25Q128 	
*********************************************************************************************************/
u16 W25QxxReadID(void)
{
  u16 ID = 0;                                 //用来存放芯片的ID
                    
  CLR_W25Qxx_CS();                            //选中W25Qxx的片选
                    
  SPI2ReadWriteByte(0x90);                     //发送读取ID命令    
  SPI2ReadWriteByte(0x00);         
  SPI2ReadWriteByte(0x00);         
  SPI2ReadWriteByte(0x00);         
  ID |= SPI2ReadWriteByte(0xFF) << 8;          //将第一次读取到的字节左移8位并赋给ID高八位
  ID |= SPI2ReadWriteByte(0xFF);               //将第二次读取到的字节赋给ID低八位
  SET_W25Qxx_CS();                            //取消W25Qxx的片选
        
  return ID;                                  //返回读取到的芯片的ID
}

/*********************************************************************************************************
* 函数名称: W25QxxRead
* 函数功能: 读取SPI FLASH，在指定地址开始读取指定长度的数据 
* 输入参数: pBuffer:数据存储区，ReadAddr:开始读取的地址(24bit)，NumByteToRead:要读取的字节数(最大65535)
* 输出参数: pBuffer
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void W25QxxRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)   
{ 
  u16 i;
  
  CLR_W25Qxx_CS();                            //选中W25Qxx的片选
  
  SPI2ReadWriteByte(W25X_READ_DATA);           //发送读取命令   
  SPI2ReadWriteByte((u8)((ReadAddr) >> 16));   //发送24bit地址    
  SPI2ReadWriteByte((u8)((ReadAddr) >> 8));   
  SPI2ReadWriteByte((u8)ReadAddr);   
  
  for(i = 0; i < NumByteToRead; i++)
  { 
    pBuffer[i] = SPI2ReadWriteByte(0XFF);      //循环读数  
  }
  
  SET_W25Qxx_CS();                            //取消W25Qxx的片选
} 

/*********************************************************************************************************
* 函数名称: W25QxxWrite
* 函数功能: 写SPI FLASH，在指定地址开始写入指定长度的数据，该函数带擦除操作 
* 输入参数: pBuffer:数据存储区，WriteAddr:开始写入的地址(24bit)，NumByteToWrite:要写入的字节数(最大65535)
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/ 
void W25QxxWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)   
{ 
  u32 secPos;                                                   //用于存放首地址WriteAddr所在的扇区
  u16 secOff;                                                   //用于存放首地址WriteAddr在扇区的偏移
  u16 secRemain;                                                //用于存放扇区剩余的长度
  u16 i;                                                        //循环计数器
                                
  secPos = WriteAddr / 4096;                                    //扇区地址
  secOff = WriteAddr % 4096;                                    //在扇区内的偏移
  secRemain = 4096 - secOff;                                    //扇区剩余空间大小   
                                
  if(NumByteToWrite <= secRemain)                               //判断要写入数据长度是否超过扇区所剩下的长度
  {                               
    secRemain = NumByteToWrite;                                 //不大于4096个字节，不会跨扇区
  } 
    
  while(1)  
  { 
    W25QxxRead(SPI_FLASH_BUF, secPos * 4096, 4096);             //读出整个扇区的内容，保存在SPI_FLASH_BUF中
    for(i = 0; i < secRemain; i++)                              //校验数据
    {
      if(SPI_FLASH_BUF[secOff + i] != 0XFF)                     //判断扇区剩余的空间是否有遗留数据
      {
        break;                                                  //有遗留数据，需要擦除 
      }                                                         
    }                                                                                                                      
    if(i < secRemain)                                           //如果需要擦除
    {                                                           
      W25QxxEraseSector(secPos);                                //擦除这个扇区
                              
      for(i = 0; i < secRemain; i++)                            //复制
      {
        SPI_FLASH_BUF[i + secOff] = pBuffer[i];                 //将遗留数据存放在pBuffer中
      }
      
      W25QxxWriteNoCheck(SPI_FLASH_BUF, secPos * 4096, 4096);   //写入整个扇区  
    }
    else
    {
      W25QxxWriteNoCheck(pBuffer, WriteAddr, secRemain);        //写已经擦除了的,直接写入扇区剩余区间.
    }

    if(NumByteToWrite == secRemain)
    {
      break;                                                    //写入结束了
    }                                                         
    else                                                        //写入未结束
    {                                                         
      secPos++;                                                 //扇区地址增1
      secOff = 0;                                               //偏移位置为0
                            
      pBuffer += secRemain;                                     //指针偏移
      WriteAddr += secRemain;                                   //写地址偏移  
      NumByteToWrite -= secRemain;                              //字节数递减
                                
      if(NumByteToWrite > 4096)                         
      {                         
        secRemain = 4096;                                       //下一个扇区还是写不完
      }                         
      else                          
      {                         
        secRemain = NumByteToWrite;                             //下一个扇区可以写完了
      }
    }
  }
} 

/*********************************************************************************************************
* 函数名称: W25QXXWakeUp
* 函数功能: 唤醒
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void W25QxxWakeUp(void)   
{  
  CLR_W25Qxx_CS();                              //选中W25Qxx的片选
  SPI2ReadWriteByte(W25X_RELEASE_POWER_DOWN);    //发送释放掉电模式命令
  SET_W25Qxx_CS();                              //取消W25Qxx的片选
  DelayNus(3);                                  //延时3us，等待TRES1
} 

/*********************************************************************************************************
* 函数名称: W25QXXEraseChip
* 函数功能: 擦除整个芯片 
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: W25X16:25s 
*           W25X32:40s 
*           W25X64:40s 
*           等待时间超长
*********************************************************************************************************/
void W25QxxEraseChip(void)   
{                                             
  W25QxxWriteEnable();                  //写使能，即将WEL置位
  W25QxxWaitBusy();                     //等待空闲   
  CLR_W25Qxx_CS();                      //选中W25Qxx的片选
  SPI2ReadWriteByte(W25X_CHIP_ERASE);    //发送片擦除命令  
  SET_W25Qxx_CS();                      //取消W25Qxx的片选

  W25QxxWaitBusy();                     //等待芯片擦除完成
}  
 
/*********************************************************************************************************
* 函数名称: W25QXXEraseSector
* 函数功能: 擦除一个扇区 
* 输入参数: Dst_Addr:扇区地址 0~511 for w25x16
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 擦除一个扇区的最少时间:150ms
*********************************************************************************************************/
void W25QxxEraseSector(u32 Dst_Addr)   
{   
  Dst_Addr *= 4096;
  W25QxxWriteEnable();                        //写使能，即将WEL置位
  W25QxxWaitBusy();                           //等待空闲
  CLR_W25Qxx_CS();                            //选中W25Qxx的片选
  SPI2ReadWriteByte(W25X_SECTOR_ERASE);        //发送扇区擦除指令 
  SPI2ReadWriteByte((u8)((Dst_Addr) >> 16));   //发送24bit地址    
  SPI2ReadWriteByte((u8)((Dst_Addr) >> 8));   
  SPI2ReadWriteByte((u8)Dst_Addr);  
  SET_W25Qxx_CS();                            //取消W25Qxx的片选
  W25QxxWaitBusy();                           //等待擦除完成
}  
