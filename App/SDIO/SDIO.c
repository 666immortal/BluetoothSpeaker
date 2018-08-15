/*********************************************************************************************************
* 模块名称: SDIO.c
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
#include "SDIO.h"
#include "UART.h"  
#include "SysIT.h"   
#include "string.h"   

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
//SDIO相关标志位
#define SDIO_FLAG_CCRCFAIL                  ((uint32_t)0x00000001)
#define SDIO_FLAG_DCRCFAIL                  ((uint32_t)0x00000002)
#define SDIO_FLAG_CTIMEOUT                  ((uint32_t)0x00000004)
#define SDIO_FLAG_DTIMEOUT                  ((uint32_t)0x00000008)
#define SDIO_FLAG_TXUNDERR                  ((uint32_t)0x00000010)
#define SDIO_FLAG_RXOVERR                   ((uint32_t)0x00000020)
#define SDIO_FLAG_CMDREND                   ((uint32_t)0x00000040)
#define SDIO_FLAG_CMDSENT                   ((uint32_t)0x00000080)
#define SDIO_FLAG_DATAEND                   ((uint32_t)0x00000100)
#define SDIO_FLAG_STBITERR                  ((uint32_t)0x00000200)
#define SDIO_FLAG_DBCKEND                   ((uint32_t)0x00000400)
#define SDIO_FLAG_CMDACT                    ((uint32_t)0x00000800)
#define SDIO_FLAG_TXACT                     ((uint32_t)0x00001000)
#define SDIO_FLAG_RXACT                     ((uint32_t)0x00002000)
#define SDIO_FLAG_TXFIFOHE                  ((uint32_t)0x00004000)
#define SDIO_FLAG_RXFIFOHF                  ((uint32_t)0x00008000)
#define SDIO_FLAG_TXFIFOF                   ((uint32_t)0x00010000)
#define SDIO_FLAG_RXFIFOF                   ((uint32_t)0x00020000)
#define SDIO_FLAG_TXFIFOE                   ((uint32_t)0x00040000)
#define SDIO_FLAG_RXFIFOE                   ((uint32_t)0x00080000)
#define SDIO_FLAG_TXDAVL                    ((uint32_t)0x00100000)
#define SDIO_FLAG_RXDAVL                    ((uint32_t)0x00200000)
#define SDIO_FLAG_SDIOIT                    ((uint32_t)0x00400000)
#define SDIO_FLAG_CEATAEND                  ((uint32_t)0x00800000)

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
//用于SD卡初始化的结构体
SDIO_CmdInitTypeDef   SDIO_CmdInitStructure;
SDIO_DataInitTypeDef  SDIO_DataInitStructure;  

//SD_ReadDisk/SD_WriteDisk函数专用buf,当这两个函数的数据缓存区地址不是4字节对齐的时候,
//需要用到该数组,确保数据缓存区地址是4字节对齐的.
__align(4) u8 SDIO_DATA_BUFFER[512];  

static u8 CardType = SDIO_STD_CAPACITY_SD_CARD_V1_1;     //SD卡类型（默认为1.x卡）
static u32 CSD_Tab[4], CID_Tab[4], RCA = 0;              //SD卡CSD,CID以及相对地址(RCA)数据
//工作模式,注意,工作模式必须通过SD_SetDeviceMode,后才算数.这里只是定义一个默认的模式(SD_DMA_MODE)
static u8 DeviceMode = SD_DMA_MODE;               
static u8 StopCondition = 0;                             //是否发送停止传输标志位,DMA多块读写的时候用到  
volatile SD_Error TransferError = SD_OK;                 //数据传输错误标志,DMA读写时使用      
volatile u8 TransferEnd = 0;                             //传输结束标志,DMA读写时使用
SD_CardInfo SDCardInfo;                                  //SD卡信息

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static SD_Error CmdError(void);                                   //检查CMD0的执行状态
static SD_Error CmdResp7Error(void);                              //检查R7响应的错误状态
static SD_Error CmdResp1Error(u8 cmd);                            //检查R1响应的错误状态
static SD_Error CmdResp3Error(void);                              //检查R3响应的错误状态
static SD_Error CmdResp2Error(void);                              //检查R2响应的错误状态
static SD_Error CmdResp6Error(u8 cmd, u16*prca);                  //检查R6响应的错误状态
static SD_Error SDEnWideBus(u8 enx);                              //SDIO使能宽总线模式
static SD_Error IsCardProgramming(u8 *pstatus);                   //检查卡是否正在执行写操作
static SD_Error FindSCR(u16 rca, u32 *pscr);                      //查找SD卡的SCR寄存器值
static u8 Convert2PowerofTwo(u16 NumberOfBytes);                  //得到NumberOfBytes以2为底的指数.
//SDIO中断处理函数，处理SDIO传输过程中的各种中断事务
static SD_Error ProcessSD_IRQSrc(void);      
//卡上电，查询所有SDIO接口上的卡设备,并查询其电压和配置时钟  
static SD_Error SD_PowerON(void);                                                   
static void SetSDIOClock(u8 clkdiv);                              //SDIO时钟初始化设置    
static SD_Error InitializeSDCards(void);                          //初始化所有的卡,并让卡进入就绪状态    
static SD_Error GetSDCardInfo(SD_CardInfo *cardinfo);             //得到卡信息        
   
static SD_Error EnableSDWideBusOperation(u32 wmode);              //设置SDIO总线宽度(MMC卡不支持4bit模式)           
static SD_Error SetSDDeviceMode(u32 mode);                        //设置SD卡工作模式  
static SD_Error SelectSDDeselect(u32 addr);                       //选卡    
static SD_Error SendSDStatus(uint32_t *pcardstatus);              //读取当前卡状态     

static SD_Error ReadSDBlock(u8 *buf,long long addr,u16 blksize);                   //SD卡读取一个块 
static SD_Error ReadSDMultiBlocks(u8 *buf,long long  addr,u16 blksize,u32 nblks);  //SD卡读取多个块 
static SD_Error WriteSDBlock(u8 *buf,long long addr,  u16 blksize);                //SD卡写1个块 
static SD_Error WriteSDMultiBlocks(u8 *buf,long long addr,u16 blksize,u32 nblks);  //SD卡写多个块  

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: CmdError
* 函数功能: 检查CMD0的执行状态
* 输入参数: void 
* 输出参数: void 
* 返 回 值: sd卡错误码
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error CmdError(void)
{
  SD_Error errorstatus = SD_OK;                       //重置SD_Error状态
  u32 timeout = SDIO_CMD0TIMEOUT;                     //超时标志，等待响应超时 
                    
  while(timeout--)                                    //超时标志递减
  {
    if(SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) != RESET)//获取SD卡的命令发送标志状态
    {
      break;                                          //命令已发送(无需响应)   
    }                  
  }                        
  if(timeout == 0)                                    //超时标志达到0，即超时
  {                  
    return SD_CMD_RSP_TIMEOUT;                        //SD卡等待响应超时
  }                  
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);                  //清除所有标记
  
  return errorstatus;                                 //返回SD卡的状态
}   

/*********************************************************************************************************
* 函数名称: CmdResp7Error
* 函数功能: 检查R7响应的错误状态
* 输入参数: void 
* 输出参数: void 
* 返 回 值: sd卡错误码
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error CmdResp7Error(void)
{
  SD_Error errorstatus = SD_OK;                   //重置SD_Error状态
  u32 status;                                     //用来存放SD卡的STA寄存器状态
  u32 timeout = SDIO_CMD0TIMEOUT;                 //超时标志，等待响应超时 
                                  
   while(timeout--)                               //超时标志递减
  {               
    status = SDIO->STA;                           //获取STA寄存器的状态
    if(status & ((1 << 0) | (1 << 2) | (1 << 6))) //CRC错误/命令响应超时/已经收到响应(CRC校验成功)
    { 
      break;                                      //跳出函数
    }
  }
   if((timeout == 0) || (status & (1 << 2)))      //响应超时
  {                                                
    errorstatus = SD_CMD_RSP_TIMEOUT;             //当前卡不是2.0兼容卡,或者不支持设定的电压范围
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);           //清除命令响应超时标志
    
    return errorstatus;                           //返回SD卡的状态
  }       
  if(status & 1 << 6)                             //成功接收到响应
  {                       
    errorstatus = SD_OK;                          //SD卡状态正常
    SDIO_ClearFlag(SDIO_FLAG_CMDREND);            //清除响应标志
  }
  
  return errorstatus;                             //返回SD卡的状态
}    

/*********************************************************************************************************
* 函数名称: CmdResp1Error
* 函数功能: 检查R1响应的错误状态
* 输入参数: cmd:当前命令
* 输出参数: void 
* 返 回 值: sd卡错误码
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error CmdResp1Error(u8 cmd)
{    
  u32 status;                                             //用来存放SD卡的STA寄存器状态
  
  while(1)
  {
    status = SDIO->STA;                                   //获取STA寄存器的状态
    if(status & ((1 << 0) | (1 << 2) | (1 << 6)))         //CRC错误/命令响应超时/已经收到响应(CRC校验成功)
    {
      break;                                              //跳出函数      
    }
  } 
  if(SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET)     //响应超时
  {                                            
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);                   //清除命令响应超时标志
    
    return SD_CMD_RSP_TIMEOUT;                            //返回等待响应超时
  }  
  if(SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET)     //CRC错误
  {                                            
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);                   //清除标志
    
    return SD_CMD_CRC_FAIL;                               //返回CRC错误
  }    
  if(SDIO->RESPCMD != cmd)                                //接收到的命令不匹配
  {
    return SD_ILLEGAL_CMD;                                //命令不匹配 
  }
  SDIO->ICR = 0X5FF;                                      //清除标记
  
  return (SD_Error)(SDIO->RESP1 & SD_OCR_ERRORBITS);      //返回卡响应
}

/*********************************************************************************************************
* 函数名称: CmdResp3Error
* 函数功能: 检查R3响应的错误状态
* 输入参数: void 
* 输出参数: void 
* 返 回 值: 错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error CmdResp3Error(void)
{
  u32 status;                                           //用来存放SD卡的STA寄存器状态
  
  while(1)
  {
    status = SDIO->STA;                                 //获取STA寄存器的状态
    if(status & ((1 << 0) | (1 << 2) | (1 << 6)))       //CRC错误/命令响应超时/已经收到响应(CRC校验成功)  
    {
      break;                                            //跳出函数
    }
  }
   if(SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET)  //响应超时
  {                       
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);                 //清除命令响应超时标志
    
    return SD_CMD_RSP_TIMEOUT;                          //返回等待响应超时
  }   
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);                    //清除所有标记
  
   return SD_OK;                                        //SD卡状态正常           
}

/*********************************************************************************************************
* 函数名称: CmdResp2Error
* 函数功能: 检查R2响应的错误状态
* 输入参数: void 
* 输出参数: void 
* 返 回 值: 错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error CmdResp2Error(void)
{
  SD_Error errorstatus = SD_OK;                       //重置SD_Error状态
  u32 status;                                         //用来存放SD卡的STA寄存器状态
  u32 timeout = SDIO_CMD0TIMEOUT;                     //超时标志，等待响应超时
  
  while(timeout--)                                    //超时标志递减
  {
    status = SDIO->STA;                               //获取STA寄存器的状态
    
    if(status & ((1 << 0) | (1 << 2) | (1 << 6)))     //CRC错误/命令响应超时/已经收到响应(CRC校验成功)  
    {
      break;                                          //跳出函数                         
    }
  }
  if((timeout == 0) || (status & (1 << 2)))           //响应超时
  {                                            
    errorstatus = SD_CMD_RSP_TIMEOUT;                 //返回等待响应超时
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);               //清除命令响应超时标志
    
    return errorstatus;                               //返回SD卡的状态
  }   
  if(SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET) //CRC错误
  {                   
    errorstatus = SD_CMD_CRC_FAIL;                    //返回CRC错误
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);               //清除响应标志
  }
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);                  //清除所有标记
  
  return errorstatus;                                 //返回SD卡的状态
} 

/*********************************************************************************************************
* 函数名称: CmdResp6Error
* 函数功能: 检查R6响应的错误状态
* 输入参数: cmd:之前发送的命令，prca:卡返回的RCA地址
* 输出参数: void 
* 返 回 值: 错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error CmdResp6Error(u8 cmd, u16*prca)
{
  SD_Error errorstatus = SD_OK;                         //重置SD_Error状态
  u32 status;                                           //用来存放SD卡的STA寄存器状态            
  u32 rspr1;                                            //用来存放响应
  
  while(1)
  {
    status = SDIO->STA;                                 //获取STA寄存器的状态
    
    if(status & ((1 << 0) | (1 << 2) | (1 << 6)))       //CRC错误/命令响应超时/已经收到响应(CRC校验成功) 
    {
      break;                                            //跳出函数
    }
  }
  if(SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET)   //响应超时
  {                                            
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);                 //清除命令响应超时标志
    
    return SD_CMD_RSP_TIMEOUT;                          //返回等待响应超时
  }      
  if(SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET)   //CRC错误
  {                   
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);                 //清除响应标志
    
    return SD_CMD_CRC_FAIL;                             //返回CRC错误
  }
  if(SDIO->RESPCMD != cmd)                              //判断是否响应cmd命令
  {
    return SD_ILLEGAL_CMD;                              //命令不匹配 
  } 
  
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);                    //清除所有标记  
  rspr1 = SDIO->RESP1;                                  //得到响应，并存放在rspr1中
  
  if(SD_ALLZERO == (rspr1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)))
  {
    *prca = (u16)(rspr1 >> 16);                         //右移16位得到,rca
    
    return errorstatus;                                 //返回SD卡的状态
  }
  if(rspr1 & SD_R6_GENERAL_UNKNOWN_ERROR)
  {
    return SD_GENERAL_UNKNOWN_ERROR;                    //返回产生未知错误
  }
  
  if(rspr1 & SD_R6_ILLEGAL_CMD)
  {
    return SD_ILLEGAL_CMD;                              //返回命令不匹配
  }
  
  if(rspr1 & SD_R6_COM_CRC_FAILED)
  {
    return SD_COM_CRC_FAILED;                           //返回CRC错误
  }
    
  return errorstatus;                                   //返回SD卡的状态
}

/*********************************************************************************************************
* 函数名称: SDEnWideBus
* 函数功能: SDIO使能宽总线模式
* 输入参数: enx:0,不使能;1,使能;
* 输出参数: void 
* 返 回 值: 错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error SDEnWideBus(u8 enx)
{
  SD_Error errorstatus = SD_OK;                                       //重置SD_Error状态
  u32 scr[2] = {0, 0};                                          
  u8 arg = 0X00;
  
  if(enx)
  {
    arg = 0X02;
  }
  else 
  {
    arg = 0X00;
  }
  
  if(SDIO->RESP1 & SD_CARD_LOCKED)                                    //检测SD卡有没有上锁                        
  {
    return SD_LOCK_UNLOCK_FAILED;                                     //返回SD卡处于LOCKED状态        
  }     
        
  errorstatus = FindSCR(RCA,scr);                                     //得到SCR寄存器数据
        
  if(errorstatus != SD_OK)                                            //如果SD卡的状态异常
  {     
    return errorstatus;                                               //返回SD卡的状态
  }     
  if((scr[1] & SD_WIDE_BUS_SUPPORT) != SD_ALLZERO)                    //支持宽总线
  {
    SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;       //设置命令参数 
    //命令号CMD55+RCA,指定下个命令为特定应用命令，不是标准命令
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;             
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;        //设置响应类型为短响应
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;                   //设置等待类型为无等待状态
    //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;               
    SDIO_SendCommand(&SDIO_CmdInitStructure);                         //调用函数，把写命令进命令寄存器
    
    errorstatus = CmdResp1Error(SD_CMD_APP_CMD);                      //等待R1响应，即数据线上有busy信号 
    
    if(errorstatus != SD_OK)                                          //R1响应错误
    {   
      return errorstatus;                                             //返回SD卡的状态
    }

    SDIO_CmdInitStructure.SDIO_Argument = arg;                        //设置命令参数 
    //命令号ACMD6，定义数据总线宽度('00'=1bit'10'=4bit)
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH; 
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;        //设置响应类型为短响应
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;                   //设置等待类型为无等待状态
    //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;               
    SDIO_SendCommand(&SDIO_CmdInitStructure);                         //调用函数，把写命令进命令寄存器 

    errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);          //等待R1响应
    
    return errorstatus;                                               //返回SD卡的状态
  }
  else 
  {
    return SD_REQUEST_NOT_APPLICABLE;                                 //不支持宽总线设置    
  }
}  

/*********************************************************************************************************
* 函数名称: IsCardProgramming
* 函数功能: 检查卡是否正在执行写操作
* 输入参数: pstatus:当前状态.
* 输出参数: void 
* 返 回 值: 错误代码
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error IsCardProgramming(u8 *pstatus)
{
  vu32 respR1 = 0;                                            //用来存放R1响应
  vu32 status = 0;                                            //用来存放SD卡的STA寄存器状态   
  
  SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16; //卡相对地址参数
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;   //发送CMD13，选定卡通过CMD线发送它的状态寄存器 
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;  //设置响应类型为短响应
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;             //设置等待类型为无等待状态
  //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;         
  SDIO_SendCommand(&SDIO_CmdInitStructure);                   //调用函数，把写命令进命令寄存器

  status = SDIO->STA;                                         //获取STA寄存器的状态
  
  while(!(status & ((1 << 0) | (1 << 6) | (1 << 2))))         //CRC错误/命令响应超时/已经收到响应(CRC校验成功) 
  {
    status = SDIO->STA;                                       //等待操作完成
  }
  
  if(SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET)         //CRC检测失败
  {  
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);                       //清除错误标记
    
    return SD_CMD_CRC_FAIL;                                   //返回CRC错误
  }
  if(SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET)         //命令超时 
  {
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);                       //清除错误标记
    
    return SD_CMD_RSP_TIMEOUT;                                //返回等待响应超时
  }
  if(SDIO->RESPCMD != SD_CMD_SEND_STATUS)
  {
    return SD_ILLEGAL_CMD;                                    //返回指令不匹配
  }
  
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);                          //清除所有标记
  
  respR1 = SDIO->RESP1;                                       //取出寄存器RESP1的状态
  
  *pstatus = (u8)((respR1 >> 9) & 0x0000000F);
  
  return SD_OK;                                               //返回SD卡状态正常
}

/*********************************************************************************************************
* 函数名称: FindSCR
* 函数功能: 查找SD卡的SCR寄存器值
* 输入参数: rca:卡相对地址，pscr:数据缓存区(存储SCR内容)
* 输出参数: void 
* 返 回 值: 错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/   
static SD_Error FindSCR(u16 rca, u32 *pscr)
{ 
  u32 index = 0;                
  SD_Error errorstatus = SD_OK;                               //重置SD_Error状态
  u32 tempscr[2] = {0, 0};      
   
  SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)8;          //设置命令参数  
  //命令号CMD16，对于标准SD卡，设置块命令长度；对于SDHC卡，块命令长度固定为512字节
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;  
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;  //设置响应类型为短响应
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;             //设置等待类型为无等待状态
  //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;          
  SDIO_SendCommand(&SDIO_CmdInitStructure);                   //调用函数，把写命令进命令寄存器

  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);           //等待R1响应

  if(errorstatus != SD_OK)                                    //R1响应错误
  {                                                           
    return errorstatus;                                       //返回响应错误   
  }

  SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16; //设置命令参数 
  //命令号CMD55,指定下个命令为特定应用命令，不是标准命令
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;        
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;  //设置响应类型为短响应
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;             //设置等待类型为无等待状态
  //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;         
  SDIO_SendCommand(&SDIO_CmdInitStructure);                   //调用函数，把写命令进命令寄存器 
  
  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);                //等待R1响应，即数据线上有busy信号
                                                              
  if(errorstatus != SD_OK)                                    //R1响应错误
  {                                                           
    return errorstatus;                                       //返回响应错误
  }

  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;           //设置数据传输超时标志
  SDIO_DataInitStructure.SDIO_DataLength = 8;                         //设置数据的长度为8
  SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_8b;  //设置数据块大小为8位
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;  //设置数据传输方向为从主机到卡
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block; //设置数据传输模式为数据块模式
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;                //设置数据通道状态机控制为开启
  SDIO_DataConfig(&SDIO_DataInitStructure);                           //根据参数初始化SD卡数据发送和接收参数

  SDIO_CmdInitStructure.SDIO_Argument = 0x0;                          //设置命令参数 
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_SEND_SCR;       //命令号ACMD51，读取配置寄存器SCR
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;          //设置响应类型为短响应  
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;                     //设置等待类型为无等待状态
  //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;                 
  SDIO_SendCommand(&SDIO_CmdInitStructure);                           //调用函数，把写命令进命令寄存器 
      
  errorstatus = CmdResp1Error(SD_CMD_SD_APP_SEND_SCR);                //等待R1响应，即数据线上有busy信号
                                                                      
  if(errorstatus != SD_OK)                                            //R1响应错误
  {                                                                   
    return errorstatus;                                               //返回响应错误
  }
  
  while(!(SDIO->STA & (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | 
      SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
  { 
    if(SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)                 //接收FIFO数据可用
    {                                                                 
      *(tempscr+index) = SDIO_ReadData();                             //读取FIFO内容
      index++;                                                        
                                                                      
      if(index >= 2)                                                  
      {                                                               
        break;                                                        
      }                                                               
    }                                                                 
  }                                                                   
  if(SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)                 //数据超时错误
  {                                                                   
    SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);                               //清错误标志
                                                                      
    return SD_DATA_TIMEOUT;                                           //返回等待数据超时
  }                                                                   
  else if(SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)            //数据块CRC错误
  {                                                                   
    SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);                               //清错误标志
                                                                      
    return SD_DATA_CRC_FAIL;                                          //返回CRC错误
  }                                                                   
  else if(SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)             //接收FIFO上溢错误
  {                                                                   
    SDIO_ClearFlag(SDIO_FLAG_RXOVERR);                                //清错误标志
                                                                      
    return SD_RX_OVERRUN;                                             //返回接收FIFO上溢错误
  }                                                                   
  else if(SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)            //接收起始位错误
  {                                                                   
    SDIO_ClearFlag(SDIO_FLAG_STBITERR);                               //清错误标志
                                                                      
    return SD_START_BIT_ERR;                                          //返回起始位错误
  }                                                                   
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);                                  //清除所有标记
  //把数据顺序按8位为单位倒过来.     
  *(pscr+1) = ((tempscr[0] & SD_0TO7BITS) << 24) | ((tempscr[0] & SD_8TO15BITS) << 8) | 
    ((tempscr[0] & SD_16TO23BITS) >> 8) | ((tempscr[0] & SD_24TO31BITS) >> 24);
  *(pscr) = ((tempscr[1] & SD_0TO7BITS) << 24) | ((tempscr[1] & SD_8TO15BITS) << 8) | 
    ((tempscr[1] & SD_16TO23BITS) >> 8) | ((tempscr[1] & SD_24TO31BITS) >> 24);
    
  return errorstatus;                                                 //返回SD卡状态
}

/*********************************************************************************************************
* 函数名称: SDIO_IRQHandler
* 函数功能: SDIO中断服务函数    
* 输入参数: void 
* 输出参数: void 
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void SDIO_IRQHandler(void) 
{                      
  ProcessSD_IRQSrc();                   //处理所有SDIO相关中断
}     

/*********************************************************************************************************
* 函数名称: ProcessSD_IRQSrc
* 函数功能: SDIO中断处理函数，处理SDIO传输过程中的各种中断事务
* 输入参数: void 
* 输出参数: void 
* 返 回 值: 错误代码
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error ProcessSD_IRQSrc(void)
{
  if(SDIO->STA & (1 << 8))                                                //接收完成中断
  {   
    if (StopCondition == 1)                                               //停止条件成立
    {
      SDIO_CmdInitStructure.SDIO_Argument = 0;                            //设置命令参数          
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;     //发送CMD12，强制停止传输数据
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;          //设置响应类型为短响应
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;                     //设置等待类型为无等待状态
      //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;                 
      SDIO_SendCommand(&SDIO_CmdInitStructure);                           //调用函数，把写命令进命令寄存器   
                                                                          
      TransferError = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);            //返回R1响应结果
    }
    else 
    {
      TransferError = SD_OK;  
    }
    SDIO->ICR |= 1 << 8;                                                      //清除完成中断标记
    SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | 
                    (1 << 4) | (1 << 5) | (1 << 9));                          //关闭相关中断
    TransferEnd = 1;
    
    return TransferError;
  }
  if(SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)                         //数据CRC错误
  {
    SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);                                       //清错误标志
    SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | 
                    (1 << 4) | (1 << 5) | (1 << 9));                          //关闭相关中断
    TransferError = SD_DATA_CRC_FAIL;
    
    return SD_DATA_CRC_FAIL;                                                  //返回数据CRC错误
  }
  if(SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)                         //数据超时错误
  {
    SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);                                       //清中断标志
    SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | 
                    (1 << 4) | (1 << 5) | (1 << 9));                          //关闭相关中断
    TransferError = SD_DATA_TIMEOUT;
    
    return SD_DATA_TIMEOUT;                                                   //返回数据传输超时
  }
  if(SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)                          //FIFO上溢错误
  {
    SDIO_ClearFlag(SDIO_FLAG_RXOVERR);                                        //清中断标志
    SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | 
                    (1 << 4) | (1 << 5) | (1 << 9));                          //关闭相关中断
    TransferError = SD_RX_OVERRUN;
    
    return SD_RX_OVERRUN;                                                     //返回接收FIFO上溢错误
  }
  if(SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET)                         //FIFO下溢错误
  {
    SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);                                       //清中断标志
    SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | 
                    (1 << 4) | (1 << 5) | (1 << 9));                          //关闭相关中断
    TransferError = SD_TX_UNDERRUN;
    
    return SD_TX_UNDERRUN;                                                    //返回FIFO下溢错误
  }
  if(SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)                         //起始位错误
  {
    SDIO_ClearFlag(SDIO_FLAG_STBITERR);                                       //清中断标志
    SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | 
                    (1 << 4) | (1 << 5) | (1 << 9));                          //关闭相关中断
    TransferError = SD_START_BIT_ERR;
    
    return SD_START_BIT_ERR;                                                  //返回起始位错误
  }
  
  return SD_OK;                                                               //SD卡状态正常
}

/*********************************************************************************************************
* 函数名称: Convert2PowerofTwo
* 函数功能: 得到NumberOfBytes以2为底的指数.
* 输入参数: NumberOfBytes:字节数.
* 输出参数: void 
* 返 回 值: 以2为底的指数值
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static u8 Convert2PowerofTwo(u16 NumberOfBytes)
{
  u8 count = 0;                         //用来存放结果
              
  while(NumberOfBytes != 1)             //只要字节数还不等于1
  {           
    NumberOfBytes >>= 1;                //字节数右移1位，相当于除以2
    count++;                            //每右移一位，count加1
  }           
              
  return count;                         //返回转化后的结果，以2为底的指数值
}    

/*********************************************************************************************************
* 函数名称: SD_PowerON
* 函数功能: 卡上电，查询所有SDIO接口上的卡设备,并查询其电压和配置时钟
* 输入参数: void 
* 输出参数: void 
* 返 回 值: 错误代码;(0,无错误)
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error SD_PowerON(void)
{
  u8  i = 0;                                        //循环计数器
  u32 response = 0, count = 0, validvoltage = 0;
  u32 SDType = SD_STD_CAPACITY;                     //SD卡的类型
  SD_Error errorstatus = SD_OK;                     //重置SD_Error状态
  SDIO_InitTypeDef  SDIO_InitStructure;             //定义结构SDIO_InitStructure,用来配置SDIO基本参数
 
  //初始化时的时钟不能大于400KHz 
  // HCLK = 72MHz, SDIOCLK = 72MHz, SDIO_CK = HCLK/(178 + 2) = 400 KHz
  SDIO_InitStructure.SDIO_ClockDiv = USER_DEFINE_SDIO_INIT_CLK_DIV;  //设置初始化分频系数
  SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;         //上升沿采集数据
  //不使用bypass模式，直接用HCLK进行分频得到SDIO_CK
  SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;  
  SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;  //空闲时不关闭时钟电源
  SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;                     //暂时配置成1位数据模式
  //硬件流，若开启，在FIFO不能进行发送和接收数据时，数据传输暂停
  SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;  
  SDIO_Init(&SDIO_InitStructure);                                 //根据参数初始化SD基本参数
 
  SDIO_SetPowerState(SDIO_PowerState_ON);                         //上电状态,开启卡时钟    
  SDIO_ClockCmd(ENABLE);                                          //SDIOCK使能 
  
   for(i = 0; i < 74; i++)
  {
    SDIO_CmdInitStructure.SDIO_Argument = 0x0;                    //设置命令参数
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE;   //命令号CMD0，复位所有的卡到空闲状态
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;       //设置响应类型为无响应
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;               //设置等待类型为无等待状态
    //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。 
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;           
    SDIO_SendCommand(&SDIO_CmdInitStructure);                     //调用函数，把写命令进命令寄存器
    
    errorstatus = CmdError();                                     //检查CMD0的执行状态
                                        
    if(errorstatus == SD_OK)                                      //如果SD卡的状态正确
    {                                   
      break;                                                      //跳出函数
    }                                   
   }                                    
   if(errorstatus)                                                //如果发生了错误
  {                                   
    return errorstatus;                                           //返回错误状态
  }

  SDIO_CmdInitStructure.SDIO_Argument = SD_CHECK_PATTERN;         //设置命令参数
  //命令号CMD8，发送SD卡接口条件，包含主机支持的电压信息，并询问卡是否支持
  SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SEND_IF_COND;        
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;      //设置响应类型为短响应
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;                 //设置等待类型为无等待状态
  //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。 
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;             
  SDIO_SendCommand(&SDIO_CmdInitStructure);                       //调用函数，把写命令进命令寄存器
      
  errorstatus = CmdResp7Error();                                  //等待R7响应，即RCA寄存器响应
  if(errorstatus == SD_OK)                                        //R7响应正常，有响应则SD卡遵循协议2.0版本
  {   
    CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0;                    //SD卡的类型为SD协议2.0版本
    SDType = SD_HIGH_CAPACITY;                                    //SD卡为高容量卡
  }   
  SDIO_CmdInitStructure.SDIO_Argument = 0x00;                     //设置命令参数 
  //命令号CMD55,指定下个命令为特定应用命令，不是标准命令
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;           
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;      //设置响应类型为短响应
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;                 //设置等待类型为无等待状态
  //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。 
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;             
  SDIO_SendCommand(&SDIO_CmdInitStructure);                       //调用函数，把写命令进命令寄存器
      
  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);                    //等待R1响应，即数据线上有busy信号 
  if(errorstatus == SD_OK)                                        //R1响应正常
  {                                  
    //SD卡,发送ACMD41 SD_APP_OP_COND,参数为:0x80100000 
    while((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
    {                            
      SDIO_CmdInitStructure.SDIO_Argument = 0x00;                 //设置命令参数 
      //命令号CMD55,指定下个命令为特定应用命令，不是标准命令
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;       
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;  //设置响应类型为短响应
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;             //设置等待类型为无等待状态
      //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;         
      SDIO_SendCommand(&SDIO_CmdInitStructure);                   //调用函数，把写命令进命令寄存器 
        
      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);                //等待R1响应，即数据线上有busy信号
      if(errorstatus != SD_OK)                                    //R1响应错误
      { 
        return errorstatus;                                       //返回响应错误
      }
      
      //ACMD41，命令参数由支持的电压范围及HCS位组成，HCS位置一来区分卡是SDSC还是SDHC
      SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_SD | SDType;  //设置命令参数 
      //命令号ACMD41，主机要求卡发送它的支持信息（HCS）和OCR寄存器内容
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_OP_COND;          
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;            //设置响应类型为短响应
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;                       //设置等待类型为无等待状态
      //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;                   
      SDIO_SendCommand(&SDIO_CmdInitStructure);                             //调用函数，把写命令进命令寄存器 
      
      errorstatus = CmdResp3Error();                              //等待R3响应，OCR寄存器的值作为ACMD41的响应
        
      if(errorstatus != SD_OK)                                    //R3响应错误
      { 
        return errorstatus;                                       //返回响应错误  
      } 
        
      response = SDIO->RESP1;;                                    //得到响应
      validvoltage = (((response >> 31) == 1) ? 1 : 0);           //判断SD卡上电是否完成
      count++;                                                    //计算重复上电的次数
    } 
    if(count >= SD_MAX_VOLT_TRIAL)                                //如果次数大于最大次数限制
    {                                                             
      errorstatus = SD_INVALID_VOLTRANGE;                         //SD卡上电失败
      
      return errorstatus;                                         //返回错误状态
    }   
    if(response &= SD_HIGH_CAPACITY)                            
    {
      CardType = SDIO_HIGH_CAPACITY_SD_CARD;                      //高容量SD卡                 
    }
  }
  else                                                            //如果是MMC卡
  {
    //MMC卡,发送CMD1 SDIO_SEND_OP_COND,参数为:0x80FF8000 
    while((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
    {                                       
      SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_MMC;//设置命令参数  
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_OP_COND;  //命令号CMD1
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;  //设置响应类型为短响应
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;             //设置等待类型为无等待状态
      //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;         
      SDIO_SendCommand(&SDIO_CmdInitStructure);                   //调用函数，把写命令进命令寄存器 
      
      errorstatus = CmdResp3Error();                              //等待R3响应   
      if(errorstatus != SD_OK)                                    //R3响应错误
      {
        return errorstatus;                                       //返回响应错误  
      }
      response = SDIO->RESP1;;                                    //得到响应
      validvoltage = (((response >> 31) == 1) ? 1 : 0);           //判断SD卡上电是否完成
      count++;                                                    //计算重复上电的次数
    }
    if(count >= SD_MAX_VOLT_TRIAL)                                //如果次数大于最大次数限制
    {
      errorstatus = SD_INVALID_VOLTRANGE;                         //SD卡上电失败
      
      return errorstatus;                                         //返回错误状态
    }
    CardType = SDIO_MULTIMEDIA_CARD;                              //SD卡为MM卡
  }
  
  return errorstatus;                                             //返回SD卡状态
}

/*********************************************************************************************************
* 函数名称: SetSDIOClock
* 函数功能: SDIO时钟初始化设置
* 输入参数: clkdiv:时钟分频系数
* 输出参数: void 
* 返 回 值: 错误代码;(0,无错误)
* 创建日期: 2018年03月01日
* 注    意: CK时钟=SDIOCLK/[clkdiv+2];(SDIOCLK时钟固定为48Mhz)
*********************************************************************************************************/
static void SetSDIOClock(u8 clkdiv)
{
  u32 tmpreg = SDIO->CLKCR;                                      //取出寄存器的值
                                                                 
  tmpreg &= 0XFFFFFF00;                                          //将低两位置0
  tmpreg |= clkdiv;                                              //将传入的分频值写入tmpreg
  SDIO->CLKCR = tmpreg;                                          //将最后的结果写入寄存器
} 
 
/*********************************************************************************************************
* 函数名称: InitializeSDCards
* 函数功能: 初始化所有的卡,并让卡进入就绪状态
* 输入参数: void 
* 输出参数: void 
* 返 回 值: 错误代码
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error InitializeSDCards(void)
{
  SD_Error errorstatus = SD_OK;                                  //重置SD_Error状态
  u16 rca = 0x01;                                       
      
  if(SDIO_GetPowerState() == 0)   
  {   
    return SD_REQUEST_NOT_APPLICABLE;                            //检查电源状态,确保为上电状态
  }   
      
  if(SDIO_SECURE_DIGITAL_IO_CARD != CardType)                    //判断卡的类型，如果不是SECURE_DIGITAL_IO_CARD
  {   
    SDIO_CmdInitStructure.SDIO_Argument = 0x0;                   //设置命令参数     
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ALL_SEND_CID;   //发送CMD2,取得CID
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;    //设置响应类型为长响应
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;              //设置等待类型为无等待状态
    //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;       
    SDIO_SendCommand(&SDIO_CmdInitStructure);                    //调用函数，把写命令进命令寄存器 
        
    errorstatus = CmdResp2Error();                               //等待R2响应   
        
    if(errorstatus != SD_OK)                                     //R2响应错误
    {   
      return errorstatus;                                        //返回响应错误        
    }
    //将返回的CID信息存储起来
    CID_Tab[0] = SDIO->RESP1;
    CID_Tab[1] = SDIO->RESP2;
    CID_Tab[2] = SDIO->RESP3;
    CID_Tab[3] = SDIO->RESP4;
  }
  
  if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || 
    (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))//判断卡类型
  {
    SDIO_CmdInitStructure.SDIO_Argument = 0x00;                  //设置命令参数 
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR;   //发送CMD3，通知所有卡发布新RCA
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;   //设置响应类型为短响应
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;              //设置等待类型为无等待状态
    //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;         
    SDIO_SendCommand(&SDIO_CmdInitStructure);                    //调用函数，把写命令进命令寄存器 
                                                                 
    errorstatus = CmdResp6Error(SD_CMD_SET_REL_ADDR, &rca);      //等待R6响应 
                                                                 
    if(errorstatus != SD_OK)                                     //R6响应错误
    {                                                            
      return errorstatus;                                        //返回响应错误        
    }
  }
  
  if(SDIO_MULTIMEDIA_CARD==CardType)
  {
    SDIO_CmdInitStructure.SDIO_Argument = (u32)(rca << 16);      //设置命令参数 
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR;   //发送CMD3，通知所有卡发布新RCA
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;   //设置响应类型为短响应
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;              //设置等待类型为无等待状态
    //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;          
    SDIO_SendCommand(&SDIO_CmdInitStructure);                    //调用函数，把写命令进命令寄存器   
      
    errorstatus = CmdResp2Error();                               //等待R2响应   
      
    if(errorstatus != SD_OK)                                     //R2响应错误
    { 
      return errorstatus;                                        //返回响应错误   
    }
  }
  //判断卡的类型，如果不是SECURE_DIGITAL_IO_CARD
  if(SDIO_SECURE_DIGITAL_IO_CARD != CardType)                     
  {
    RCA = rca;
    SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)(rca << 16);  //设置命令参数 
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_CSD;        //发送CMD9+卡RCA,取得CSD
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;     //设置响应类型为长响应
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;               //设置等待类型为无等待状态
    //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;           
    SDIO_SendCommand(&SDIO_CmdInitStructure);                     //调用函数，把写命令进命令寄存器   
    
    errorstatus = CmdResp2Error();                                //等待R2响应  
    if(errorstatus != SD_OK)                                      //R2响应错误
    {
      return errorstatus;                                         //返回响应错误        
    }
    //将返回的CID信息存储起来
    CSD_Tab[0] = SDIO->RESP1;
    CSD_Tab[1] = SDIO->RESP2;
    CSD_Tab[2] = SDIO->RESP3;            
    CSD_Tab[3] = SDIO->RESP4;              
  }
  
  return SD_OK;                                                   //卡初始化成功
} 

/*********************************************************************************************************
* 函数名称: SD_GetCardInfo
* 函数功能: 得到卡信息
* 输入参数: cardinfo:卡信息存储区
* 输出参数: void 
* 返 回 值: 错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error GetSDCardInfo(SD_CardInfo *cardinfo)
{
  SD_Error errorstatus = SD_OK;                                    //重置SD_Error状态
  u8 tmp = 0;     
  
  cardinfo->CardType = (u8)CardType;                               //卡类型
  cardinfo->RCA = (u16)RCA;                                        //卡RCA值
  tmp = (u8)((CSD_Tab[0] & 0xFF000000) >> 24);
  cardinfo->SD_csd.CSDStruct = (tmp & 0xC0) >> 6;                  //CSD结构
  //2.0协议还没定义这部分(为保留),应该是后续协议定义的
  cardinfo->SD_csd.SysSpecVersion = (tmp & 0x3C) >> 2;  
  cardinfo->SD_csd.Reserved1 = tmp & 0x03;                         //2个保留位  
  tmp = (u8)((CSD_Tab[0] & 0x00FF0000) >> 16);                     //第1个字节
  cardinfo->SD_csd.TAAC = tmp;                                     //数据读时间1
  tmp = (u8)((CSD_Tab[0]&0x0000FF00)>>8);                          //第2个字节
  cardinfo->SD_csd.NSAC = tmp;                                     //数据读时间2
  tmp = (u8)(CSD_Tab[0] & 0x000000FF);                             //第3个字节
  cardinfo->SD_csd.MaxBusClkFrec = tmp;                            //传输速度     
  tmp = (u8)((CSD_Tab[1] & 0xFF000000) >> 24);                     //第4个字节
  cardinfo->SD_csd.CardComdClasses = tmp << 4;                     //卡指令类高四位
  tmp = (u8)((CSD_Tab[1] & 0x00FF0000) >> 16);                     //第5个字节
  cardinfo->SD_csd.CardComdClasses |= (tmp & 0xF0) >> 4;           //卡指令类低四位
  cardinfo->SD_csd.RdBlockLen = tmp & 0x0F;                        //最大读取数据长度
  tmp = (u8)((CSD_Tab[1] & 0x0000FF00) >> 8);                      //第6个字节
  cardinfo->SD_csd.PartBlockRead = (tmp & 0x80) >> 7;              //允许分块读
  cardinfo->SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6;            //写块错位
  cardinfo->SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5;            //读块错位
  cardinfo->SD_csd.DSRImpl = (tmp & 0x10) >> 4;
  cardinfo->SD_csd.Reserved2 = 0;                                  //保留
  if((CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0) || 
      (SDIO_MULTIMEDIA_CARD == CardType))                          //标准1.1/2.0卡/MMC卡
  {
    cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;              //C_SIZE(12位)
    tmp = (u8)(CSD_Tab[1] & 0x000000FF);                           //第7个字节  
    cardinfo->SD_csd.DeviceSize |= (tmp) << 2;
    tmp = (u8)((CSD_Tab[2] & 0xFF000000) >> 24);                   //第8个字节  
    cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;
    cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
    cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);
    tmp = (u8)((CSD_Tab[2] & 0x00FF0000) >> 16);                   //第9个字节  
    cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
    cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
    cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;            
    tmp = (u8)((CSD_Tab[2] & 0x0000FF00) >> 8);                    //第10个字节  
    cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;
    cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1);    //计算卡容量
    cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
    cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen);  //块大小
    cardinfo->CardCapacity *= cardinfo->CardBlockSize;
  }
  else if(CardType == SDIO_HIGH_CAPACITY_SD_CARD)                  //高容量卡
  {
    tmp = (u8)(CSD_Tab[1] & 0x000000FF);                           //第7个字节  
    cardinfo->SD_csd.DeviceSize = (tmp & 0x3F)<<16;                 
    tmp = (u8)((CSD_Tab[2] & 0xFF000000) >> 24);                   //第8个字节  
    cardinfo->SD_csd.DeviceSize |= (tmp << 8);                     
    tmp = (u8)((CSD_Tab[2] & 0x00FF0000) >> 16);                   //第9个字节  
    cardinfo->SD_csd.DeviceSize |= (tmp);                          
    tmp = (u8)((CSD_Tab[2] & 0x0000FF00) >> 8);                    //第10个字节  
    cardinfo->CardCapacity = (long long)(cardinfo->SD_csd.DeviceSize + 1) * 512 * 1024;//计算卡容量
    cardinfo->CardBlockSize = 512;                                 //块大小固定为512字节
  }    
  cardinfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
  cardinfo->SD_csd.EraseGrMul = (tmp & 0x3F) << 1;     
  tmp = (u8)(CSD_Tab[2] & 0x000000FF);                             //第11个字节  
  cardinfo->SD_csd.EraseGrMul |= (tmp & 0x80) >> 7;
  cardinfo->SD_csd.WrProtectGrSize = (tmp & 0x7F);
  tmp = (u8)((CSD_Tab[3] & 0xFF000000) >> 24);                     //第12个字节  
  cardinfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
  cardinfo->SD_csd.ManDeflECC = (tmp & 0x60) >> 5;
  cardinfo->SD_csd.WrSpeedFact = (tmp & 0x1C) >> 2;
  cardinfo->SD_csd.MaxWrBlockLen = (tmp & 0x03) << 2;   
  tmp = (u8)((CSD_Tab[3] & 0x00FF0000) >> 16);                     //第13个字节
  cardinfo->SD_csd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
  cardinfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
  cardinfo->SD_csd.Reserved3 = 0;
  cardinfo->SD_csd.ContentProtectAppli = (tmp & 0x01);  
  tmp = (u8)((CSD_Tab[3] & 0x0000FF00) >> 8);                      //第14个字节
  cardinfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
  cardinfo->SD_csd.CopyFlag = (tmp & 0x40) >> 6;
  cardinfo->SD_csd.PermWrProtect = (tmp & 0x20) >> 5;
  cardinfo->SD_csd.TempWrProtect = (tmp & 0x10) >> 4;
  cardinfo->SD_csd.FileFormat = (tmp & 0x0C) >> 2;
  cardinfo->SD_csd.ECC = (tmp & 0x03);  
  tmp = (u8)(CSD_Tab[3] & 0x000000FF);                             //第15个字节
  cardinfo->SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
  cardinfo->SD_csd.Reserved4 = 1;     
  tmp = (u8)((CID_Tab[0] & 0xFF000000) >> 24);                     //第0个字节
  cardinfo->SD_cid.ManufacturerID = tmp;                        
  tmp = (u8)((CID_Tab[0] & 0x00FF0000) >> 16);                     //第1个字节
  cardinfo->SD_cid.OEM_AppliID=tmp<<8;                    
  tmp = (u8)((CID_Tab[0] & 0x000000FF00) >> 8);                    //第2个字节
  cardinfo->SD_cid.OEM_AppliID |= tmp;      
  tmp = (u8)(CID_Tab[0] & 0x000000FF);                             //第3个字节  
  cardinfo->SD_cid.ProdName1 = tmp << 24;                            
  tmp = (u8)((CID_Tab[1] & 0xFF000000) >> 24);                     //第4个字节
  cardinfo->SD_cid.ProdName1 |= tmp << 16;                      
  tmp = (u8)((CID_Tab[1] & 0x00FF0000) >> 16);                     //第5个字节
  cardinfo->SD_cid.ProdName1 |= tmp << 8;                      
  tmp = (u8)((CID_Tab[1] & 0x0000FF00) >> 8);                      //第6个字节
  cardinfo->SD_cid.ProdName1 |= tmp;                        
  tmp = (u8)(CID_Tab[1] & 0x000000FF);                             //第7个字节
  cardinfo->SD_cid.ProdName2 = tmp;                          
  tmp = (u8)((CID_Tab[2] & 0xFF000000) >> 24);                     //第8个字节
  cardinfo->SD_cid.ProdRev = tmp;                      
  tmp = (u8)((CID_Tab[2] & 0x00FF0000) >> 16);                     //第9个字节
  cardinfo->SD_cid.ProdSN = tmp << 24;                      
  tmp = (u8)((CID_Tab[2] & 0x0000FF00) >> 8);                      //第10个字节
  cardinfo->SD_cid.ProdSN |= tmp << 16;                      
  tmp = (u8)(CID_Tab[2] & 0x000000FF);                             //第11个字节
  cardinfo->SD_cid.ProdSN |= tmp<<8;                        
  tmp = (u8)((CID_Tab[3] & 0xFF000000) >> 24);                     //第12个字节
  cardinfo->SD_cid.ProdSN |= tmp;                            
  tmp = (u8)((CID_Tab[3] & 0x00FF0000) >> 16);                     //第13个字节
  cardinfo->SD_cid.Reserved1 |= (tmp & 0xF0) >> 4;
  cardinfo->SD_cid.ManufactDate = (tmp & 0x0F) << 8;    
  tmp = (u8)((CID_Tab[3] & 0x0000FF00) >> 8);                      //第14个字节
  cardinfo->SD_cid.ManufactDate |= tmp;         
  tmp = (u8)(CID_Tab[3] & 0x000000FF);                             //第15个字节
  cardinfo->SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
  cardinfo->SD_cid.Reserved2 = 1;   
  
  return errorstatus;                                              //返回SD卡状态
}

/*********************************************************************************************************
* 函数名称: EnableSDWideBusOperation
* 函数功能: 设置SDIO总线宽度(MMC卡不支持4bit模式)
* 输入参数: wmode:位宽模式.0,1位数据宽度;1,4位数据宽度;2,8位数据宽度
* 输出参数: void 
* 返 回 值: SD卡错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error EnableSDWideBusOperation(u32 wmode)
{
  SD_Error errorstatus = SD_OK;           //重置SD_Error状态
  
  if(SDIO_MULTIMEDIA_CARD == CardType)    //如果SD卡是MM卡
  {
    return SD_UNSUPPORTED_FEATURE;        //MMC卡不支持
  }
  else if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||
          (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
  {
    if(wmode >= 2)                        //如果位宽模式为8位数据宽度
    {
      return SD_UNSUPPORTED_FEATURE;      //不支持8位模式
    }
    else   
    {
      errorstatus = SDEnWideBus(wmode);   //使能SD卡宽总线模式
      if(SD_OK == errorstatus)            //如果SD卡状态正常
      {
        SDIO->CLKCR &= ~(3 << 11);        //清除之前的位宽设置    
        SDIO->CLKCR |= (u16)wmode << 11;  //1位/4位总线宽度 
        SDIO->CLKCR |= 0 << 14;           //不开启硬件流控制
      }
    }  
  }
  
  return errorstatus;                     //返回SD卡状态
}

/*********************************************************************************************************
* 函数名称: SetSDDeviceMode
* 函数功能: 设置SD卡工作模式
* 输入参数: Mode:0，轮询模式；1，DMA模式
* 输出参数: void 
* 返 回 值: 错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error SetSDDeviceMode(u32 Mode)
{
  SD_Error errorstatus = SD_OK;                         //重置SD_Error状态
  
  if((Mode == SD_DMA_MODE) || (Mode == SD_POLLING_MODE))//如果输入的参数正确
  {
    DeviceMode = Mode;                                  //设置传入的模式
  }               
  else                
  {               
    errorstatus = SD_INVALID_PARAMETER;                 //输入状态错误
  }               
                  
  return errorstatus;                                   //返回SD卡状态   
}

/*********************************************************************************************************
* 函数名称: SelectSDDeselect
* 函数功能: 选卡
* 输入参数: 发送CMD7,选择相对地址(rca)为addr的卡,取消其他卡.如果为0,则都不选择.addr:卡的RCA地址
* 输出参数: void 
* 返 回 值: 错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error SelectSDDeselect(u32 addr)
{
  SDIO_CmdInitStructure.SDIO_Argument =  addr;                   //设置命令参数 
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEL_DESEL_CARD;   //发送CMD7，选择相对地址(rca)为addr的卡
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;     //设置响应类型为短响应
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;                //设置等待类型为无等待状态
  //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;            
  SDIO_SendCommand(&SDIO_CmdInitStructure);                      //调用函数，把写命令进命令寄存器   
  
  return CmdResp1Error(SD_CMD_SEL_DESEL_CARD);                   //返回R1响应结果
}

/*********************************************************************************************************
* 函数名称: SendSDStatus
* 函数功能: 读取当前卡状态
* 输入参数: pcardstatus:卡状态
* 输出参数: void 
* 返 回 值: 错误代码
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error SendSDStatus(uint32_t *pcardstatus)
{
  SD_Error errorstatus = SD_OK;                                 //重置SD_Error状态
                          
  if(pcardstatus == NULL)                                       //如果卡状态为0
  {                       
    errorstatus = SD_INVALID_PARAMETER;                         //参数错误
    return errorstatus;                                         //返回SD卡的状态
  }

  SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;   //设置命令参数      
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;     //发送CMD13，选定卡通过CMD线发送它的状态寄存器
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;    //设置响应类型为短响应
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;               //设置等待类型为无等待状态
  //开启命令通道状态机控制，CPSM在开始发送命令之前等待数据传输结束。
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;           
  SDIO_SendCommand(&SDIO_CmdInitStructure);                     //调用函数，把写命令进命令寄存器   

  errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);              //等待R1响应状态 
  
  if(errorstatus != SD_OK)                                      //R2响应错误
  {                                                             
    return errorstatus;                                         //返回响应错误   
  }
  *pcardstatus = SDIO->RESP1;                                   //读取响应值
  
  return errorstatus;                                           //返回SD卡状态
} 

/*********************************************************************************************************
* 函数名称: ReadSDBlock
* 函数功能: SD卡读取一个块 
* 输入参数: buf:读数据缓存区(必须4字节对齐!!),addr:读取地址,blksize:块大小
* 输出参数: void 
* 返 回 值: 错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error ReadSDBlock(u8 *buf, long long addr, u16 blksize)
{    
  SD_Error errorstatus = SD_OK;                                       //重置SD_Error状态
  u8 power;                                                           
  u32 count = 0, *tempbuff = (u32*)buf;                               //转换为u32指针 
  u32 timeout = SDIO_DATATIMEOUT;                                     
                
  if(NULL == buf)                                                     //如果读数缓冲区为空
  {             
    return SD_INVALID_PARAMETER;                                      //参数错误
  }             
                
  SDIO->DCTRL = 0x0;                                                  //数据控制寄存器清零(关DMA)
                  
  if(CardType == SDIO_HIGH_CAPACITY_SD_CARD)                          //SD卡为大容量卡
  {             
    blksize = 512;                                                    //块大小设置为512
    addr >>= 9;                                                       //读取地址右移9位
  }   
 
  SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_1b;  //设置数据块大小为1位
  SDIO_DataInitStructure.SDIO_DataLength = 0;                         //设置数据的长度为0
  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;           //设置数据传输超时标志
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;                //设置数据通道状态机控制为开启
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;  //设置数据传输方向为从主机到卡
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block; //设置数据传输模式为数据块模式
  SDIO_DataConfig(&SDIO_DataInitStructure);                           //根据参数初始化SD卡数据发送和接收参数
  
  if(SDIO->RESP1 & SD_CARD_LOCKED)
  {
    return SD_LOCK_UNLOCK_FAILED;                                     //卡锁了
  }
  
  if((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
  {
    power = Convert2PowerofTwo(blksize);      
 
    SDIO_CmdInitStructure.SDIO_Argument =  blksize;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);                         //发送CMD16+设置数据长度为blksize,短响应
                    
    errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);                 //等待R1响应  
                    
    if(errorstatus != SD_OK)                
    {               
      return errorstatus;                                             //响应错误   
    }
  }
  else 
  {
    return SD_INVALID_PARAMETER;                              
  }
  
  SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4 ;   //清除DPSM状态机配置
  SDIO_DataInitStructure.SDIO_DataLength = blksize ;
  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataConfig(&SDIO_DataInitStructure);
  
  SDIO_CmdInitStructure.SDIO_Argument =  addr;
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);                  //发送CMD17+从addr地址出读取数据,短响应 
                                                             
  errorstatus = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);     //等待R1响应   
                                                             
  if(errorstatus != SD_OK)                                   
  {                                                          
    return errorstatus;                                      //响应错误   
  }                                                          
  if(DeviceMode == SD_POLLING_MODE)                          //查询模式,轮询数据   
  {                                                          
    INTX_DISABLE();                                          //关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
    //无上溢/CRC/超时/完成(标志)/起始位错误
    while(!(SDIO->STA & ((1 << 5) | (1 << 1) | (1 << 3) | (1 << 10) | (1 << 9))))
    {
      if(SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)    //接收区半满,表示至少存了8个字
      {
        for(count = 0; count < 8; count++)                   //循环读取数据
        {
          *(tempbuff + count) = SDIO->FIFO;
        }
        tempbuff += 8;   
        timeout = 0X7FFFFF;                                  //读数据溢出时间
      }
      else                                                   //处理超时
      {
        if(timeout == 0)
        {
          return SD_DATA_TIMEOUT;
        }
        timeout--;
      }
    } 
    if(SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)      //数据超时错误
    {                       
      SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);                    //清错误标志
      return SD_DATA_TIMEOUT;
     }
    else if(SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) //数据块CRC错误
    {
      SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);                    //清错误标志
      return SD_DATA_CRC_FAIL;       
    }
    else if(SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)  //接收fifo上溢错误
    {
      SDIO_ClearFlag(SDIO_FLAG_RXOVERR);                     //清错误标志
      return SD_RX_OVERRUN;     
    }
    else if(SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) //接收起始位错误
    {
      SDIO_ClearFlag(SDIO_FLAG_STBITERR);                    //清错误标志
      return SD_START_BIT_ERR;     
    }   
    while(SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)     //FIFO里面,还存在可用数据
    {
      *tempbuff = SDIO_ReadData();                           //循环读取数据
      tempbuff++;
    }
    INTX_ENABLE();                                           //开启总中断
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);                       //清除所有标记
  }
  else if(DeviceMode == SD_DMA_MODE)
  {
    ConfigSDDMA((u32*)buf, blksize, DMA_DIR_PeripheralSRC); 
    TransferError = SD_OK;
    StopCondition = 0;                                       //单块读,不需要发送停止传输指令
    TransferEnd = 0;                                         //传输结束标置位，在中断服务置1
    SDIO->MASK |= (1 << 1) | (1 << 3) | (1 << 8) | (1 << 5) | (1 << 9);  //配置需要的中断 
    SDIO_DMACmd(ENABLE);
    while(((DMA2->ISR & 0X2000) == RESET) && (TransferEnd == 0) && (TransferError == SD_OK) && timeout)
    {
      timeout--;                                             //等待传输完成 
    }
    if(timeout == 0)
    {
      return SD_DATA_TIMEOUT;                                //超时
    }
    if(TransferError != SD_OK)
    {
      errorstatus = TransferError;
    }
  }   
  
  return errorstatus;                                        //返回SD卡状态
}

/*********************************************************************************************************
* 函数名称: ReadSDMultiBlocks
* 函数功能: SD卡读取多个块 
* 输入参数: buf:读数据缓存区,addr:读取地址,blksize:块大小,nblks:要读取的块数
* 输出参数: void 
* 返 回 值: 错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
__align(4) u32 *tempbuff;
static SD_Error ReadSDMultiBlocks(u8 *buf, long long addr, u16 blksize, u32 nblks)
{
  SD_Error errorstatus = SD_OK;                            //重置SD_Error状态
  u8 power;
  u32 count = 0;
  u32 timeout = SDIO_DATATIMEOUT;  
  tempbuff = (u32*)buf;                                    //转换为u32指针
        
  SDIO->DCTRL = 0x0;                                       //数据控制寄存器清零(关DMA)   
  if(CardType == SDIO_HIGH_CAPACITY_SD_CARD)               //大容量卡
  {
    blksize = 512;
    addr >>= 9;
  }  
 
    SDIO_DataInitStructure.SDIO_DataBlockSize = 0;         //清除DPSM状态机配置
    SDIO_DataInitStructure.SDIO_DataLength = 0 ;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT ;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);
  
  if(SDIO->RESP1 & SD_CARD_LOCKED)
  {
    return SD_LOCK_UNLOCK_FAILED;                          //卡锁了
  }
  if((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
  {
    power = Convert2PowerofTwo(blksize);
    
 
    SDIO_CmdInitStructure.SDIO_Argument =  blksize;        //发送CMD16+设置数据长度为blksize,短响应 
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);      //等待R1响应   
    if(errorstatus != SD_OK)
    {
      return errorstatus;                                  //响应错误   
    }
  }
  else 
  {
    return SD_INVALID_PARAMETER;    
  }
  if(nblks>1)                                              //多块读  
  {                      
     if(nblks * blksize > SD_MAX_DATA_LENGTH)
    {
      return SD_INVALID_PARAMETER;                         //判断是否超过最大接收长度
    }
    SDIO_DataInitStructure.SDIO_DataBlockSize = power<<4;  //nblks*blksize,512块大小,卡到控制器
    SDIO_DataInitStructure.SDIO_DataLength = nblks*blksize ;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT ;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);

    SDIO_CmdInitStructure.SDIO_Argument =  addr;           //发送CMD18+从addr地址出读取数据,短响应 
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_MULT_BLOCK;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);  
    
    errorstatus = CmdResp1Error(SD_CMD_READ_MULT_BLOCK);   //等待R1响应   
    if(errorstatus != SD_OK)
    {
      return errorstatus;                                  //响应错误    
    }
    if(DeviceMode == SD_POLLING_MODE)
    {
      INTX_DISABLE();                                      //关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
      while(!(SDIO->STA & ((1 << 5) | (1 << 1) | (1 << 3) | (1 << 8) | (1 << 9))))//无上溢/CRC/超时/完成(标志)/起始位错误
      {
        if(SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)//接收区半满,表示至少存了8个字
        {
          for(count = 0; count < 8; count++)               //循环读取数据
          {
            *(tempbuff+count) = SDIO->FIFO;
          }
          tempbuff += 8;   
          timeout = 0X7FFFFF;                              //读数据溢出时间
        }
        else                                               //处理超时
        {
          if(timeout == 0)return SD_DATA_TIMEOUT;
          timeout--;
        }
      }  
      if(SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)       //数据超时错误
      {                       
        SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);                     //清错误标志
        return SD_DATA_TIMEOUT;
      }
      else if(SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)  //数据块CRC错误
      {
        SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);                     //清错误标志
        return SD_DATA_CRC_FAIL;       
      }
      else if(SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)   //接收fifo上溢错误
      {
        SDIO_ClearFlag(SDIO_FLAG_RXOVERR);                      //清错误标志
        return SD_RX_OVERRUN;     
      }
      else if(SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)  //接收起始位错误
      {
        SDIO_ClearFlag(SDIO_FLAG_STBITERR);                     //清错误标志
        return SD_START_BIT_ERR;     
      }   
        
      while(SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)      //FIFO里面,还存在可用数据
      {
        *tempbuff = SDIO_ReadData();                            //循环读取数据
        tempbuff++;
      }
      if(SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET)        //接收结束
      {
        if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||
            (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
        {
          SDIO_CmdInitStructure.SDIO_Argument =  0;             //发送CMD12+结束传输
          SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
          SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
          SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
          SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
          SDIO_SendCommand(&SDIO_CmdInitStructure);  
          
          errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);//等待R1响应  
          
          if(errorstatus != SD_OK)
          {
            return errorstatus;                                 //返回SD卡状态
          }
        }
      }
      INTX_ENABLE();                                       //开启总中断
      SDIO_ClearFlag(SDIO_STATIC_FLAGS);                   //清除所有标记
     }
    else if(DeviceMode == SD_DMA_MODE)
    {
       ConfigSDDMA((u32*)buf, nblks * blksize, DMA_DIR_PeripheralSRC); 
       TransferError = SD_OK;
       StopCondition = 1;                                  //多块读,需要发送停止传输指令 
       TransferEnd = 0;                                    //传输结束标置位，在中断服务置1
       SDIO->MASK |= (1 << 1) | (1 << 3) | (1 << 8) | (1 << 5) | (1 << 9);  //配置需要的中断 
       SDIO->DCTRL |= 1 << 3;                              //SDIO DMA使能 
       while(((DMA2->ISR & 0X2000) == RESET) && timeout)
      {
        timeout--;                                         //等待传输完成 
      }
      if(timeout == 0)
      {
        return SD_DATA_TIMEOUT;                            //超时
      }
      while((TransferEnd == 0) && (TransferError == SD_OK))
      {
        
      }        
      if(TransferError != SD_OK)
      {
        errorstatus = TransferError;     
      }
    }     
  }
  
  return errorstatus;                                      //返回SD卡状态
}  

/*********************************************************************************************************
* 函数名称: WriteSDBlock
* 函数功能: SD卡写1个块 
* 输入参数: buf:数据缓存区,addr:写地址,blksize:块大小，SDHC卡为512字节   
* 输出参数: void 
* 返 回 值: 错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static SD_Error WriteSDBlock(u8 *buf, long long addr, u16 blksize)
{
  SD_Error errorstatus = SD_OK;                     //重置SD_Error状态 
  u8  power = 0, cardstate = 0;
  u32 timeout = 0, bytestransferred = 0;
  u32 cardstatus = 0, count = 0, restwords = 0; 
  u32  tlen = blksize;                              //总长度(字节)
  u32* tempbuff = (u32*)buf;                        //将传入的数据存入tempbuff中
   
  if(buf == NULL)                                   //如果传入数据为0
  {
    return SD_INVALID_PARAMETER;                    //返回参数错误   
  }
  
  SDIO->DCTRL = 0x0;                                //数据控制寄存器清零(关DMA)   
   
  SDIO_DataInitStructure.SDIO_DataBlockSize = 0;    //清除DPSM状态机配置
  SDIO_DataInitStructure.SDIO_DataLength = 0 ;
  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT ;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataConfig(&SDIO_DataInitStructure);
  
  
  if(SDIO->RESP1 & SD_CARD_LOCKED)
  {
    return SD_LOCK_UNLOCK_FAILED;                   //返回SD卡解锁失败
  }                                                 
  if(CardType == SDIO_HIGH_CAPACITY_SD_CARD)        //如果SD卡为大容量卡
  {
    blksize = 512;
    addr >>= 9;
  }    
  if((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
  {
    power = Convert2PowerofTwo(blksize);  
    //以下这段是在ST驱动库上添加的，没有这一段容易卡死在DMA检测中
    SDIO_CmdInitStructure.SDIO_Argument = blksize;            //设置数据长度为blksize
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;//发送CMD16
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;//短响应
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);  
    
    errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN); //等待R1响应 
    
    if(errorstatus != SD_OK)                          //如果SD卡响应异常
    {
      return errorstatus;                             //返回响应错误   
    }
  }
  else 
  {
    return SD_INVALID_PARAMETER;                      //返回参数错误 
  }
 
  SDIO_CmdInitStructure.SDIO_Argument = (u32)RCA<<16; //发送CMD13,查询卡的状态,短响应   
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);  

  errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);    //等待R1响应 
  
  if(errorstatus != SD_OK)
  {
    return errorstatus;
  }
  cardstatus = SDIO->RESP1;                            
  timeout = SD_DATATIMEOUT;
  while(((cardstatus & 0x00000100) == 0) && (timeout > 0))//检查READY_FOR_DATA位是否置位
  {
    timeout--;

    SDIO_CmdInitStructure.SDIO_Argument = (u32)RCA << 16; //发送CMD13,查询卡的状态,短响应
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);  
    
    errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);      //等待R1响应          
    if(errorstatus != SD_OK)                              //SD卡状态异常
    {
      return errorstatus;                                 //返回SD卡状态        
    }
    cardstatus = SDIO->RESP1;                             //取出寄存器RESP1中的状态值
  }
  
  if(timeout == 0)                                        //等待超时
  {
    return SD_ERROR;                                      //返回SD卡错误
  }
  
  SDIO_CmdInitStructure.SDIO_Argument = addr;             //发送CMD24,写单块指令,短响应   
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_SINGLE_BLOCK;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);  
  
  errorstatus = CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK); //等待R1响应          
  if(errorstatus != SD_OK)
  {
    return errorstatus;                                   //返回SD卡状态   
  }
  StopCondition = 0;                                      //单块写,不需要发送停止传输指令 

  SDIO_DataInitStructure.SDIO_DataBlockSize = power<<4;   //blksize, 控制器到卡  
  SDIO_DataInitStructure.SDIO_DataLength = blksize ;
  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT ;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataConfig(&SDIO_DataInitStructure);
    
  timeout = SDIO_DATATIMEOUT;
  if (DeviceMode == SD_POLLING_MODE)
  {
    INTX_DISABLE();                                       //关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
    //数据块发送成功/下溢/CRC/超时/起始位错误
    while(!(SDIO->STA & ((1 << 10) | (1 << 4) | (1 << 1) | (1 << 3) | (1 << 9))))
    {
      if(SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET) //发送区半空,表示至少存了8个字
      {
        if((tlen - bytestransferred) < SD_HALFFIFOBYTES)  //不够32字节了
        {
          restwords = ((tlen - bytestransferred) % 4 == 0) ? ((tlen - bytestransferred) / 4) : ((tlen - bytestransferred) / 4 + 1);
          
          for(count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
          {
            SDIO_WriteData(*tempbuff);
          }
        }
        else
        {
          for(count = 0; count < 8; count++)
          {
            SDIO_WriteData(*(tempbuff+count));
          }
          tempbuff += 8;
          bytestransferred += 32;
        }
        timeout = 0X3FFFFFFF;                                 //写数据溢出时间
      }
      else
      {
        if(timeout == 0)return SD_DATA_TIMEOUT;
        timeout--;
      }
    } 
    if(SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)       //数据超时错误
    {                       
      SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);                     //清错误标志
      return SD_DATA_TIMEOUT;
     }
    else if(SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)  //数据块CRC错误
    {
      SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);                     //清错误标志
      return SD_DATA_CRC_FAIL;       
    }
    else if(SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET)  //接收fifo下溢错误
    {
      SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);                     //清错误标志
      return SD_TX_UNDERRUN;     
    }
    else if(SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)  //接收起始位错误
    {
      SDIO_ClearFlag(SDIO_FLAG_STBITERR);                     //清错误标志
      return SD_START_BIT_ERR;     
    }   
        
    INTX_ENABLE();                                            //开启总中断
    SDIO->ICR = 0X5FF;                                        //清除所有标记    
  }
  else if(DeviceMode == SD_DMA_MODE)
  {
    ConfigSDDMA((u32*)buf,blksize,DMA_DIR_PeripheralDST);     //SDIO DMA配置
    TransferError = SD_OK;
    StopCondition = 0;                                        //单块写,不需要发送停止传输指令 
    TransferEnd = 0;                                          //传输结束标置位，在中断服务置1
    SDIO->MASK |= (1 << 1) | (1 << 3) | (1 << 8) | (1 << 4) | (1 << 9);  //配置产生数据接收完成中断
      SDIO->DCTRL |= 1 << 3;                                  //SDIO DMA使能.  
     while(((DMA2->ISR & 0X2000) == RESET) && timeout)
    {
      timeout--;                                              //等待传输完成 
    }
    if(timeout == 0)
    {
      InitSD();                                               //重新初始化SD卡,可以解决写入死机的问题
      return SD_DATA_TIMEOUT;                                 //超时   
    }
    timeout = SDIO_DATATIMEOUT;
    while((TransferEnd == 0) && (TransferError == SD_OK) && timeout)
    {
      timeout--;
    }
     if(timeout == 0)
    {
      return SD_DATA_TIMEOUT;                                 //超时   
    }
    if(TransferError!= SD_OK)
    {
      return TransferError;
    }
  }  
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);                          //清除所有标记
  errorstatus = IsCardProgramming(&cardstate);
  
  while((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || 
          (cardstate == SD_CARD_RECEIVING)))
  {
    errorstatus = IsCardProgramming(&cardstate);
  }   
  
  return errorstatus;                                         //返回SD卡状态
}

/*********************************************************************************************************
* 函数名称: WriteSDMultiBlocks
* 函数功能: SD卡写多个块 
* 输入参数: buf:数据缓存区，addr:写地址，blksize:块大小，nblks:要写入的块数
* 输出参数: void 
* 返 回 值: 错误状态  
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/                         
static SD_Error WriteSDMultiBlocks(u8 *buf,long long addr,u16 blksize,u32 nblks)
{
  SD_Error errorstatus = SD_OK;
  u8  power = 0, cardstate = 0;
  u32 timeout = 0, bytestransferred = 0;
  u32 count = 0, restwords = 0;
  u32 tlen = nblks * blksize;                            //总长度(字节)
  u32 *tempbuff = (u32*)buf;  
  
  if(buf == NULL)
  {
    return SD_INVALID_PARAMETER;                         //参数错误
  }
  SDIO->DCTRL = 0x0;                                     //数据控制寄存器清零(关DMA)   
  
  SDIO_DataInitStructure.SDIO_DataBlockSize = 0;         //清除DPSM状态机配置  
  SDIO_DataInitStructure.SDIO_DataLength = 0 ;
  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT ;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataConfig(&SDIO_DataInitStructure);
  
  if(SDIO->RESP1 & SD_CARD_LOCKED)
  {
    return SD_LOCK_UNLOCK_FAILED;                        //卡锁了
  }
   if(CardType == SDIO_HIGH_CAPACITY_SD_CARD)            //大容量卡
  {
    blksize = 512;
    addr >>= 9;
  }    
  if((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
  {
    power = Convert2PowerofTwo(blksize);
    
    SDIO_CmdInitStructure.SDIO_Argument = blksize;       //发送CMD16+设置数据长度为blksize,短响应
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);  
    
    errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);    //等待R1响应  
    
    if(errorstatus != SD_OK)
    {
      return errorstatus;                                //返回SD卡状态
    }
    
  }
  else 
  {
    return SD_INVALID_PARAMETER;   
  }
  if(nblks > 1)
  {            
    if(nblks * blksize > SD_MAX_DATA_LENGTH)
    {
      return SD_INVALID_PARAMETER;   
    }
    if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) 
          || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
    {
      //提高性能
      SDIO_CmdInitStructure.SDIO_Argument = (u32)RCA<<16;       //发送ACMD55,短响应   
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);  
        
      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);              //等待R1响应 
        
      if(errorstatus != SD_OK)
      {
        return errorstatus;                                     //返回SD卡状态
      }
        
      SDIO_CmdInitStructure.SDIO_Argument = nblks;              //发送CMD23,设置块数量,短响应    
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCK_COUNT;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);
      
      errorstatus = CmdResp1Error(SD_CMD_SET_BLOCK_COUNT);  //等待R1响应 
        
      if(errorstatus != SD_OK)
      {
        return errorstatus;                                 //返回SD卡状态
      }    
    } 

    SDIO_CmdInitStructure.SDIO_Argument = addr;             //发送CMD25,多块写指令,短响应     
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_MULT_BLOCK;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);  

     errorstatus=CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK);    //等待R1响应          
  
    if(errorstatus != SD_OK)
    {
      return errorstatus;                                   //返回SD卡状态
    }

    SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4; //blksize, 控制器到卡  
    SDIO_DataInitStructure.SDIO_DataLength = nblks*blksize;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);
    
    if(DeviceMode == SD_POLLING_MODE)
    {
      timeout = SDIO_DATATIMEOUT;
      INTX_DISABLE();                     //关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
      
      //下溢/CRC/数据结束/超时/起始位错误
      while(!(SDIO->STA & ((1 << 4) | (1 << 1) | (1 << 8) | (1 << 3) | (1 << 9))))
      {
        if(SDIO->STA & (1 << 14))                             //发送区半空,表示至少存了8字(32字节)
        {    
          if((tlen - bytestransferred) < SD_HALFFIFOBYTES)    //不够32字节了
          {
            restwords = ((tlen - bytestransferred) % 4 == 0) ? ((tlen - bytestransferred) / 4) : 
                          ((tlen - bytestransferred) / 4 + 1);
            for(count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
            {
              SDIO_WriteData(*tempbuff);
            }
          }
          else                                                //发送区半空,可以发送至少8字(32字节)数据
          {
            for(count = 0; count < SD_HALFFIFO; count++)
            {
              SDIO_WriteData(*(tempbuff+count));
            }
            tempbuff += SD_HALFFIFO;
            bytestransferred += SD_HALFFIFOBYTES;
          }
          timeout = 0X3FFFFFFF;                               //写数据溢出时间
        }
        else
        {
          if(timeout == 0)
          {
            return SD_DATA_TIMEOUT; 
          }
          timeout--;
        }
      } 
      if(SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)     //数据超时错误
      {                       
        SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);                   //清错误标志
        
        return SD_DATA_TIMEOUT;
      }
      else if(SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)//数据块CRC错误
      {
        SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);                   //清错误标志
        
        return SD_DATA_CRC_FAIL;       
      }
      else if(SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET)//接收fifo下溢错误
      {
        SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);                   //清错误标志
        
        return SD_TX_UNDERRUN;     
      }
      else if(SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)//接收起始位错误
      {
        SDIO_ClearFlag(SDIO_FLAG_STBITERR);                   //清错误标志
        
        return SD_START_BIT_ERR;     
      }                           
      if(SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET)      //发送结束
      {                               
        if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) 
              || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
        {   
          SDIO_CmdInitStructure.SDIO_Argument =0;             //发送CMD12+结束传输     
          SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
          SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
          SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
          SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
          SDIO_SendCommand(&SDIO_CmdInitStructure);  
          
          errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);//等待R1响应   
          if(errorstatus != SD_OK)
          {
            return errorstatus;                               //返回SD卡状态
          }
        }
      }
      INTX_ENABLE();                                          //开启总中断
      SDIO_ClearFlag(SDIO_STATIC_FLAGS);                      //清除所有标记
    }
    else if(DeviceMode == SD_DMA_MODE)
    {
      ConfigSDDMA((u32*)buf, nblks * blksize, DMA_DIR_PeripheralDST);//SDIO DMA配置
      TransferError = SD_OK;
      StopCondition = 1;                                  //多块写,需要发送停止传输指令 
      TransferEnd = 0;                                    //传输结束标置位，在中断服务置1
      SDIO->MASK|=(1<<1)|(1<<3)|(1<<8)|(1<<4)|(1<<9);     //配置产生数据接收完成中断
      SDIO->DCTRL |= 1 << 3;                              //SDIO DMA使能. 
      timeout = SDIO_DATATIMEOUT;
      
      while(((DMA2->ISR & 0X2000) == RESET) && timeout)
      {
        timeout--;                                         //等待传输完成 
      }
      
      if(timeout == 0)                                     //超时
      {                    
        InitSD();                                          //重新初始化SD卡,可以解决写入死机的问题
        
        return SD_DATA_TIMEOUT;                            //超时   
      }
      timeout = SDIO_DATATIMEOUT;
      
      while((TransferEnd == 0) && (TransferError == SD_OK) && timeout)
      {
        timeout--;
      }
      if(timeout == 0)
      {
        return SD_DATA_TIMEOUT;                            //超时   
      }
      if(TransferError != SD_OK)
      {
        return TransferError;   
      }
    }
  }
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);                       //清除所有标记
  errorstatus = IsCardProgramming(&cardstate);
  
  while((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
  {
    errorstatus = IsCardProgramming(&cardstate);
  }   
  
  return errorstatus;                                      //返回SD卡状态
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitSD
* 函数功能: 初始化SD卡
* 输入参数: void 
* 输出参数: void
* 返 回 值: 错误代码;(0,无错误)
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/
SD_Error InitSD(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;  //定义结构体NVIC_InitStructure,用来配置SD卡的中断服务
  GPIO_InitTypeDef  GPIO_InitStructure; //定义结构体GPIO_InitStructure,用来配置SD卡的GPIO
  
  u8 clkDiv = 0;                        //SD卡的分区
  SD_Error errorStatus = SD_OK;         //重置SD_Error状态
  
  //SD卡时钟使能
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);//使能SDIO的时钟
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO | RCC_AHBPeriph_DMA2, ENABLE);     //使能SDIO的时钟   
  
  //设置SD卡的引脚
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;                             //复用推挽输出模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                           //设置I/O口速度为50MHz
  GPIO_Init(GPIOC, &GPIO_InitStructure);                                      //根据参数初始化PC.8~12 
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;                                   //设置SDIO的引脚PD2
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;                             //复用推挽输出模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                           //设置I/O口速度为50MHz
  GPIO_Init(GPIOD, &GPIO_InitStructure);                                      //根据参数初始化PD2

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;                                   //设置SDIO的引脚PD7
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;                               //复用推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                           //IO口速度为50MHz
  GPIO_Init(GPIOD, &GPIO_InitStructure);                                      //根据参数初始化PD7
             
  SDIO_DeInit();                                                              //对SDIO所有寄存器进行复位
 
  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;               //SDIO中断配置
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;     //抢占优先级0 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            //子优先级0 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;               //使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);                               //根据初始化外设NVIC寄存器
                                                                
  errorStatus = SD_PowerON();                                   //SD卡上电并进行卡识别流程，确认卡的操作电压
                                                                
  if(errorStatus == SD_OK)                                      
  {                                                             
    errorStatus = InitializeSDCards();                          //卡识别成功，进行卡初始化  
  }                                                             
  if(errorStatus == SD_OK)                                      
  {                                                             
    errorStatus = GetSDCardInfo(&SDCardInfo);                   //用来读取CSD/CID寄存器
  }                                                             
  if(errorStatus == SD_OK)                                      
  {                                                             
    errorStatus = SelectSDDeselect((u32)(SDCardInfo.RCA<<16));  //通过CMD7命令rca选择要操作的卡
  }                                                             
  if(errorStatus == SD_OK)                                      
  {                                                             
    errorStatus = EnableSDWideBusOperation(1);                  //为了提高读写速度，开启4位模式
  }                                                             
  if((errorStatus == SD_OK) || (SDIO_MULTIMEDIA_CARD == CardType))
  {          
    if(SDCardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1 || //如果SD协议为1.1版本或者2.0版本
      SDCardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0)
    {
      clkDiv = USER_DEFINE_SDIO_TRANSFER_CLK_DIV + 6;           //V1.1/V2.0卡，设置最高72/12=6Mhz
    }
    else 
    {
      clkDiv = USER_DEFINE_SDIO_TRANSFER_CLK_DIV;               //SDHC等其他卡，设置最高72/6=12Mhz
    }
    //设置时钟频率,SDIO时钟计算公式:SDIO_CK时钟=SDIOCLK/[clkdiv+2];其中,SDIOCLK固定为48Mhz 
    SetSDIOClock(clkDiv);  
    //SD设备有两种模式进行选择，DMA模式和轮询模式，需要时请屏蔽另一种模式
    //errorstatus = SD_SetDeviceMode(SD_DMA_MODE);              //设置为DMA模式
    errorStatus = SetSDDeviceMode(SD_POLLING_MODE);             //设置为轮询模式
  }
  if(errorStatus)                                               //如果SD卡状态错误
  {
    printf(" SD card Error!\r\n");                              //打印错误信息
    while(1)                                                    //进入死循环
    {
      
    }
  }
  
  return errorStatus;
}

/*********************************************************************************************************
* 函数名称: SD_PowerOFF
* 函数功能: SDIO电源关闭
* 输入参数: void 
* 输出参数: void 
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void SD_PowerOFF(void)
{
  SDIO_SetPowerState(SDIO_PowerState_OFF);  //SDIO电源关闭,时钟停止  
}   

/*********************************************************************************************************
* 函数名称: ConfigSDDMA
* 函数功能: 配置SDIO DMA  
* 输入参数: mbuf:存储器地址，bufsize:传输数据量，DMA_DIR:方向;
* 输出参数: void 
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: DMA_DIR_PeripheralDST ,存储器-->SDIO(写数据);DMA_DIR_PeripheralSRC,SDIO-->存储器(读数据);
*********************************************************************************************************/    
void ConfigSDDMA(u32*mbuf, u32 bufsize, u32 DMA_DIR)
{     
  DMA_InitTypeDef DMA_InitStructure;            //定义结构DMA_InitStructure,用来配置SD卡的DMA

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);                //使能DMA2时钟
              
  DMA_DeInit(DMA2_Channel4);                                        //将DMA2的通道4寄存器重设为缺省值
  DMA_Cmd(DMA2_Channel4, DISABLE);                                  //初始化之前先关闭DMA2 通道4

  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SDIO->FIFO;      //DMA外设基地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)mbuf;                 //DMA内存基地址
  DMA_InitStructure.DMA_DIR = DMA_DIR;                              //数据传输方向，从内存读取发送到外设
  DMA_InitStructure.DMA_BufferSize = bufsize/4;                     //DMA通道的DMA缓存的大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;           //内存地址寄存器递增
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;//数据宽度为32位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;   //数据宽度为32位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                     //工作在正常缓存模式
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;               //DMA通道 x拥有高优先级 
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                      //DMA通道x没有设置为内存到内存传输
  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器
  DMA_Init(DMA2_Channel4, &DMA_InitStructure);  

  DMA_Cmd(DMA2_Channel4, DISABLE);                                  //开启DMA2 通道4
}   

/*********************************************************************************************************
* 函数名称: GetSDState
* 函数功能: 返回SD卡的状态
* 输入参数: void 
* 输出参数: void 
* 返 回 值: SD卡状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
SDCardState GetSDState(void)
{
  u32 resp1 = 0;                                //用来存放RESP1寄存器的内容
  
  if(SendSDStatus(&resp1) != SD_OK)             //如果SD卡的RESP1寄存器异常
  {
    return SD_CARD_ERROR;                       //返回卡异常
  }
  else 
  {
    return (SDCardState)((resp1 >> 9) & 0x0F);  //返回SD卡的状态
  }
}

/*********************************************************************************************************
* 函数名称: ReadSDDisk
* 函数功能: 读SD卡
* 输入参数: buf:读数据缓存区，sector:扇区地址，cnt:扇区个数  
* 输出参数: void 
* 返 回 值: 错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/                 
u8 ReadSDDisk(u8*buf, u32 sector, u8 cnt)
{
  u8 sta = SD_OK;             //重置SD_Error状态
  long long lsector = sector; //存放扇区地址
  u8 n;                       //循环计数器
  
  lsector <<= 9;              //地址左移9位
  
  if((u32)buf % 4 != 0)
  {
    for(n = 0; n < cnt; n++)
    {
      sta = ReadSDBlock(SDIO_DATA_BUFFER, lsector + 512 * n, 512);
      memcpy(buf, SDIO_DATA_BUFFER, 512);
      buf += 512;
    } 
  }
  else
  {
    if(cnt == 1)                                               //扇区个数为1
    {      
      sta = ReadSDBlock(buf, lsector, 512);                    //执行单个扇区的读操作
    }
    else 
    {      
      sta = ReadSDMultiBlocks(buf, lsector, 512, cnt);         //执行多个扇区的读操作 
    }
  }
  
  return sta;                                                  //返回函数执行状态
}

/*********************************************************************************************************
* 函数名称: WriteSDDisk
* 函数功能: 写SD卡
* 输入参数: buf:写数据缓存区，sector:扇区地址，cnt:扇区个数  
* 输出参数: void 
* 返 回 值: 错误状态
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8 WriteSDDisk(u8*buf, u32 sector, u8 cnt)
{
  u8 sta = SD_OK;                 //重置SD_Error状态
  u8 n;                           //循环计数器
  long long lsector = sector;     //存放扇区地址
  
  lsector <<= 9;                  //地址左移9位
  
  if((u32)buf % 4 != 0)
  {
    for(n = 0; n < cnt; n++)
    {
      memcpy(SDIO_DATA_BUFFER, buf,512);
      sta = WriteSDBlock(SDIO_DATA_BUFFER, lsector + 512 * n, 512); 
      buf += 512;
    } 
  }
  else
  {
    if(cnt == 1)                                                //扇区个数为1
    {
      sta = WriteSDBlock(buf, lsector, 512);                    //执行单个扇区的写操作
    }
    else 
    {
      sta = WriteSDMultiBlocks(buf, lsector, 512, cnt);         //执行多个扇区的写操作
    }
  }
  
  return sta;                                                   //返回函数执行状态
}
