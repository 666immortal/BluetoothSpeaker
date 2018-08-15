/*********************************************************************************************************
* 模块名称: SDIO.h
* 摘    要: SD卡驱动
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
#ifndef _SDIO_H_
#define _SDIO_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "DataType.h"
#include "stm32f10x.h" 

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
//用户配置区
//SDIO时钟计算公式:SDIO_CK时钟=SDIOCLK/[clkdiv+2];其中,SDIOCLK一般为72Mhz
//使用DMA模式的时候,传输速率可以到24Mhz,不过如果你的卡不是高速卡,可能也会出错
//出错就请降低时钟,使用查询模式的话,推荐SDIO_TRANSFER_CLK_DIV设置为3或者更大
#define USER_DEFINE_SDIO_INIT_CLK_DIV        0xB2//用户定义SDIO初始化频率，最大400Kh  
#define USER_DEFINE_SDIO_TRANSFER_CLK_DIV    0x04//用户定义SDIO传输频率,该值太小可能会导致读写文件出错 

//SDIO工作模式定义,通过SD_SetDeviceMode函数设置.
#define SD_POLLING_MODE    	0//查询模式,该模式下,如果读写有问题,建议增大SDIO_TRANSFER_CLK_DIV的设置.
#define SD_DMA_MODE         1//DMA模式,该模式下,如果读写有问题,建议增大SDIO_TRANSFER_CLK_DIV的设置.   

//SDIO 各种错误枚举定义
typedef enum
{	 
	//特殊错误定义 
	SD_CMD_CRC_FAIL                    = (1), 
	SD_DATA_CRC_FAIL                   = (2), 
	SD_CMD_RSP_TIMEOUT                 = (3), 
	SD_DATA_TIMEOUT                    = (4), 
	SD_TX_UNDERRUN                     = (5), 
	SD_RX_OVERRUN                      = (6), 
	SD_START_BIT_ERR                   = (7), 
	SD_CMD_OUT_OF_RANGE                = (8), 
	SD_ADDR_MISALIGNED                 = (9), 
	SD_BLOCK_LEN_ERR                   = (10),
	SD_ERASE_SEQ_ERR                   = (11),
	SD_BAD_ERASE_PARAM                 = (12),
	SD_WRITE_PROT_VIOLATION            = (13),
	SD_LOCK_UNLOCK_FAILED              = (14),
	SD_COM_CRC_FAILED                  = (15),
	SD_ILLEGAL_CMD                     = (16),
	SD_CARD_ECC_FAILED                 = (17),
	SD_CC_ERROR                        = (18),
	SD_GENERAL_UNKNOWN_ERROR           = (19),
	SD_STREAM_READ_UNDERRUN            = (20),
	SD_STREAM_WRITE_OVERRUN            = (21),
	SD_CID_CSD_OVERWRITE               = (22),
	SD_WP_ERASE_SKIP                   = (23),
	SD_CARD_ECC_DISABLED               = (24),
	SD_ERASE_RESET                     = (25),
	SD_AKE_SEQ_ERROR                   = (26),
	SD_INVALID_VOLTRANGE               = (27),
	SD_ADDR_OUT_OF_RANGE               = (28),
	SD_SWITCH_ERROR                    = (29),
	SD_SDIO_DISABLED                   = (30),
	SD_SDIO_FUNCTION_BUSY              = (31),
	SD_SDIO_FUNCTION_FAILED            = (32),
	SD_SDIO_UNKNOWN_FUNCTION           = (33),
	//标准错误定义
	SD_INTERNAL_ERROR, 
	SD_NOT_CONFIGURED,
	SD_REQUEST_PENDING, 
	SD_REQUEST_NOT_APPLICABLE, 
	SD_INVALID_PARAMETER,  
	SD_UNSUPPORTED_FEATURE,  
	SD_UNSUPPORTED_HW,  
	SD_ERROR,  
	SD_OK = 0 
} SD_Error;  

//SD卡CSD寄存器数据		  
typedef struct
{
	u8  CSDStruct;            /*!< CSD structure */
	u8  SysSpecVersion;       /*!< System specification version */
	u8  Reserved1;            /*!< Reserved */
	u8  TAAC;                 /*!< Data read access-time 1 */
	u8  NSAC;                 /*!< Data read access-time 2 in CLK cycles */
	u8  MaxBusClkFrec;        /*!< Max. bus clock frequency */
	u16 CardComdClasses;      /*!< Card command classes */
	u8  RdBlockLen;           /*!< Max. read data block length */
	u8  PartBlockRead;        /*!< Partial blocks for read allowed */
	u8  WrBlockMisalign;      /*!< Write block misalignment */
	u8  RdBlockMisalign;      /*!< Read block misalignment */
	u8  DSRImpl;              /*!< DSR implemented */
	u8  Reserved2;            /*!< Reserved */
	u32 DeviceSize;           /*!< Device Size */
	u8  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min */
	u8  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max */
	u8  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min */
	u8  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max */
	u8  DeviceSizeMul;        /*!< Device size multiplier */
	u8  EraseGrSize;          /*!< Erase group size */
	u8  EraseGrMul;           /*!< Erase group size multiplier */
	u8  WrProtectGrSize;      /*!< Write protect group size */
	u8  WrProtectGrEnable;    /*!< Write protect group enable */
	u8  ManDeflECC;           /*!< Manufacturer default ECC */
	u8  WrSpeedFact;          /*!< Write speed factor */
	u8  MaxWrBlockLen;        /*!< Max. write data block length */
	u8  WriteBlockPaPartial;  /*!< Partial blocks for write allowed */
	u8  Reserved3;            /*!< Reserded */
	u8  ContentProtectAppli;  /*!< Content protection application */
	u8  FileFormatGrouop;     /*!< File format group */
	u8  CopyFlag;             /*!< Copy flag (OTP) */
	u8  PermWrProtect;        /*!< Permanent write protection */
	u8  TempWrProtect;        /*!< Temporary write protection */
	u8  FileFormat;           /*!< File Format */
	u8  ECC;                  /*!< ECC code */
	u8  CSD_CRC;              /*!< CSD CRC */
	u8  Reserved4;            /*!< always 1*/
} SD_CSD;   

//SD卡CID寄存器数据
typedef struct
{
	u8  ManufacturerID;       //生产ID号
	u16 OEM_AppliID;          //应用ID号
	u32 ProdName1;            //商品名1
	u8  ProdName2;            //商品名2
	u8  ProdRev;              //商品版本
	u32 ProdSN;               //序列号
	u8  Reserved1;            //保留地址
	u16 ManufactDate;         //生产日期
	u8  CID_CRC;              //CID_CRC
	u8  Reserved2;            //保留地址
} SD_CID;	 

//SD卡状态
typedef enum
{
	SD_CARD_READY                  = ((uint32_t)0x00000001),
	SD_CARD_IDENTIFICATION         = ((uint32_t)0x00000002),
	SD_CARD_STANDBY                = ((uint32_t)0x00000003),
	SD_CARD_TRANSFER               = ((uint32_t)0x00000004),
	SD_CARD_SENDING                = ((uint32_t)0x00000005),
	SD_CARD_RECEIVING              = ((uint32_t)0x00000006),
	SD_CARD_PROGRAMMING            = ((uint32_t)0x00000007),
	SD_CARD_DISCONNECTED           = ((uint32_t)0x00000008),
	SD_CARD_ERROR                  = ((uint32_t)0x000000FF)
}SDCardState;

//SD卡信息,包括CSD,CID等数据
typedef struct
{
  SD_CSD SD_csd;                  //SD卡的CSD，卡的操作条件信息
  SD_CID SD_cid;                  //SD卡的CID，用来识别卡的个体号码
  long long CardCapacity;         //SD卡容量,单位:字节,最大支持2^64字节大小的卡.
  u32 CardBlockSize;              //SD卡块大小
  u16 RCA;                        //SD卡相对地址：卡的本地系统地址，初始化时，动态地由卡建议，由主机核准 
  u8 CardType;                    //SD卡类型
} SD_CardInfo;                    //SD卡相关信息  

extern SD_CardInfo SDCardInfo;    //将SD卡信息拓展为外部变量


//SDIO 指令集
#define SD_CMD_GO_IDLE_STATE                       ((u8)0)
#define SD_CMD_SEND_OP_COND                        ((u8)1)
#define SD_CMD_ALL_SEND_CID                        ((u8)2)
#define SD_CMD_SET_REL_ADDR                        ((u8)3) 
#define SD_CMD_SET_DSR                             ((u8)4)
#define SD_CMD_SDIO_SEN_OP_COND                    ((u8)5)
#define SD_CMD_HS_SWITCH                           ((u8)6)
#define SD_CMD_SEL_DESEL_CARD                      ((u8)7)
#define SD_CMD_HS_SEND_EXT_CSD                     ((u8)8)
#define SD_CMD_SEND_CSD                            ((u8)9)
#define SD_CMD_SEND_CID                            ((u8)10)
#define SD_CMD_READ_DAT_UNTIL_STOP                 ((u8)11)
#define SD_CMD_STOP_TRANSMISSION                   ((u8)12)
#define SD_CMD_SEND_STATUS                         ((u8)13)
#define SD_CMD_HS_BUSTEST_READ                     ((u8)14)
#define SD_CMD_GO_INACTIVE_STATE                   ((u8)15)
#define SD_CMD_SET_BLOCKLEN                        ((u8)16)
#define SD_CMD_READ_SINGLE_BLOCK                   ((u8)17)
#define SD_CMD_READ_MULT_BLOCK                     ((u8)18)
#define SD_CMD_HS_BUSTEST_WRITE                    ((u8)19)
#define SD_CMD_WRITE_DAT_UNTIL_STOP                ((u8)20) 
#define SD_CMD_SET_BLOCK_COUNT                     ((u8)23) 
#define SD_CMD_WRITE_SINGLE_BLOCK                  ((u8)24)
#define SD_CMD_WRITE_MULT_BLOCK                    ((u8)25)
#define SD_CMD_PROG_CID                            ((u8)26)
#define SD_CMD_PROG_CSD                            ((u8)27)
#define SD_CMD_SET_WRITE_PROT                      ((u8)28)
#define SD_CMD_CLR_WRITE_PROT                      ((u8)29)
#define SD_CMD_SEND_WRITE_PROT                     ((u8)30)
#define SD_CMD_SD_ERASE_GRP_START                  ((u8)32)                                                          
#define SD_CMD_SD_ERASE_GRP_END                    ((u8)33)                                                         
#define SD_CMD_ERASE_GRP_START                     ((u8)35)
#define SD_CMD_ERASE_GRP_END                       ((u8)36)
#define SD_CMD_ERASE                               ((u8)38)
#define SD_CMD_FAST_IO                             ((u8)39)
#define SD_CMD_GO_IRQ_STATE                        ((u8)40)
#define SD_CMD_LOCK_UNLOCK                         ((u8)42)
#define SD_CMD_APP_CMD                             ((u8)55)
#define SD_CMD_GEN_CMD                             ((u8)56)
#define SD_CMD_NO_CMD                              ((u8)64)
#define SD_CMD_APP_SD_SET_BUSWIDTH                 ((u8)6) 
#define SD_CMD_SD_APP_STAUS                        ((u8)13)
#define SD_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS        ((u8)22)
#define SD_CMD_SD_APP_OP_COND                      ((u8)41)
#define SD_CMD_SD_APP_SET_CLR_CARD_DETECT          ((u8)42)
#define SD_CMD_SD_APP_SEND_SCR                     ((u8)51)
#define SD_CMD_SDIO_RW_DIRECT                      ((u8)52)
#define SD_CMD_SDIO_RW_EXTENDED                    ((u8)53)
#define SD_CMD_SD_APP_GET_MKB                      ((u8)43) 
#define SD_CMD_SD_APP_GET_MID                      ((u8)44) 
#define SD_CMD_SD_APP_SET_CER_RN1                  ((u8)45) 
#define SD_CMD_SD_APP_GET_CER_RN2                  ((u8)46) 
#define SD_CMD_SD_APP_SET_CER_RES2                 ((u8)47) 
#define SD_CMD_SD_APP_GET_CER_RES1                 ((u8)48) 
#define SD_CMD_SD_APP_SECURE_READ_MULTIPLE_BLOCK   ((u8)18) 
#define SD_CMD_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK  ((u8)25) 
#define SD_CMD_SD_APP_SECURE_ERASE                 ((u8)38) 
#define SD_CMD_SD_APP_CHANGE_SECURE_AREA           ((u8)49) 
#define SD_CMD_SD_APP_SECURE_WRITE_MKB             ((u8)48) 
  			   
//支持的SD卡定义
#define SDIO_STD_CAPACITY_SD_CARD_V1_1             ((u32)0x00000000)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0             ((u32)0x00000001)
#define SDIO_HIGH_CAPACITY_SD_CARD                 ((u32)0x00000002)
#define SDIO_MULTIMEDIA_CARD                       ((u32)0x00000003)
#define SDIO_SECURE_DIGITAL_IO_CARD                ((u32)0x00000004)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD            ((u32)0x00000005)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD          ((u32)0x00000006)
#define SDIO_HIGH_CAPACITY_MMC_CARD                ((u32)0x00000007)

//SDIO相关参数定义
#define NULL 0
#define SDIO_STATIC_FLAGS               ((u32)0x000005FF)
#define SDIO_CMD0TIMEOUT                ((u32)0x00010000)	  
#define SDIO_DATATIMEOUT                ((u32)0xFFFFFFFF)	  
#define SDIO_FIFO_Address               ((u32)0x40018080)

//Mask for errors Card Status R1 (OCR Register)  
#define SD_OCR_ADDR_OUT_OF_RANGE        ((u32)0x80000000)
#define SD_OCR_ADDR_MISALIGNED          ((u32)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR            ((u32)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR            ((u32)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM          ((u32)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION     ((u32)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED       ((u32)0x01000000)
#define SD_OCR_COM_CRC_FAILED           ((u32)0x00800000)
#define SD_OCR_ILLEGAL_CMD              ((u32)0x00400000)
#define SD_OCR_CARD_ECC_FAILED          ((u32)0x00200000)
#define SD_OCR_CC_ERROR                 ((u32)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR    ((u32)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN     ((u32)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN     ((u32)0x00020000)
#define SD_OCR_CID_CSD_OVERWRIETE       ((u32)0x00010000)
#define SD_OCR_WP_ERASE_SKIP            ((u32)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED        ((u32)0x00004000)
#define SD_OCR_ERASE_RESET              ((u32)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR            ((u32)0x00000008)
#define SD_OCR_ERRORBITS                ((u32)0xFDFFE008)

//Masks for R6 Response 
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((u32)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((u32)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((u32)0x00008000)
#define SD_VOLTAGE_WINDOW_SD            ((u32)0x80100000)
#define SD_HIGH_CAPACITY                ((u32)0x40000000)
#define SD_STD_CAPACITY                 ((u32)0x00000000)
#define SD_CHECK_PATTERN                ((u32)0x000001AA)
#define SD_VOLTAGE_WINDOW_MMC           ((u32)0x80FF8000)
#define SD_MAX_VOLT_TRIAL               ((u32)0x0000FFFF)
#define SD_ALLZERO                      ((u32)0x00000000)
#define SD_WIDE_BUS_SUPPORT             ((u32)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((u32)0x00010000)
#define SD_CARD_LOCKED                  ((u32)0x02000000)
#define SD_CARD_PROGRAMMING             ((u32)0x00000007)
#define SD_CARD_RECEIVING               ((u32)0x00000006)
#define SD_DATATIMEOUT                  ((u32)0xFFFFFFFF)
#define SD_0TO7BITS                     ((u32)0x000000FF)
#define SD_8TO15BITS                    ((u32)0x0000FF00)
#define SD_16TO23BITS                   ((u32)0x00FF0000)
#define SD_24TO31BITS                   ((u32)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((u32)0x01FFFFFF)
#define SD_HALFFIFO                     ((u32)0x00000008)
#define SD_HALFFIFOBYTES                ((u32)0x00000020)
#define SD_CCCC_LOCK_UNLOCK             ((u32)0x00000080)
#define SD_CCCC_WRITE_PROT              ((u32)0x00000040)
#define SD_CCCC_ERASE                   ((u32)0x00000020)

//CMD8指令
#define SDIO_SEND_IF_COND               ((u32)0x00000008)

/*********************************************************************************************************
*                                              结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/
SD_Error InitSD(void);                                  //初始化SD卡                     
                                                      
void SD_PowerOFF(void);                                 //SDIO电源关闭
void ConfigSDDMA(u32*mbuf, u32 bufsize, u32 DMA_DIR);   //配置SD卡的DMA     
SDCardState GetSDState(void);                           //获取SD卡的状态  
                                                      
u8 ReadSDDisk(u8*buf, u32 sector, u8 cnt);              //读SD卡
u8 WriteSDDisk(u8*buf, u32 sector, u8 cnt);             //写SD卡

#endif 
