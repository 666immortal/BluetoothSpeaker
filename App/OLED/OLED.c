/*********************************************************************************************************
* 模块名称: OLED.c   
* 摘    要: 4线串行接口，CS、DC、SCLK、SDIN、RES
* 当前版本: 1.0.0
* 作    者: 
* 完成日期: 2018年03月01日
* 内    容:
* 注    意: OLED的显存
            存放格式如下.
            [0]0 1 2 3 ... 127
            [1]0 1 2 3 ... 127
            [2]0 1 2 3 ... 127
            [3]0 1 2 3 ... 127
            [4]0 1 2 3 ... 127
            [5]0 1 2 3 ... 127
            [6]0 1 2 3 ... 127
            [7]0 1 2 3 ... 127                                                                   
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
#include  <stm32f10x_conf.h>
#include  "OLED.h"
#include  "stdlib.h"
#include  "SysTick.h"
#include  "OLEDFont.h"
#include  "UART.h"
#include  "UIicon.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/  		   
#define OLED_CMD    0    //写命令
#define OLED_DATA   1    //写数据

//片选线CS输出高电平
#define SET_OLED_CS()   GPIO_SetBits(USER_DEFINE_OLED_CS_GPIO_PORT, USER_DEFINE_OLED_CS_GPIO_PIN) 
//片选线CS输出低电平
#define CLR_OLED_CS()   GPIO_ResetBits(USER_DEFINE_OLED_CS_GPIO_PORT, USER_DEFINE_OLED_CS_GPIO_PIN)  

//硬复位线RES输出高电平
#define SET_OLED_RES()  GPIO_SetBits(USER_DEFINE_OLED_RES_GPIO_PORT, USER_DEFINE_OLED_RES_GPIO_PIN)  
//硬复位线RES输出低电平
#define CLR_OLED_RES()  GPIO_ResetBits(USER_DEFINE_OLED_RES_GPIO_PORT, USER_DEFINE_OLED_RES_GPIO_PIN)

//DC输出高电平用来读写数据  
#define SET_OLED_DC()   GPIO_SetBits(USER_DEFINE_OLED_DC_GPIO_PORT, USER_DEFINE_OLED_DC_GPIO_PIN)
//DC输出低电平用来读写命令  
#define CLR_OLED_DC()   GPIO_ResetBits(USER_DEFINE_OLED_DC_GPIO_PORT, USER_DEFINE_OLED_DC_GPIO_PIN)
   
//时钟线SCLK线输出高电平
#define SET_OLED_SCLK() GPIO_SetBits(USER_DEFINE_OLED_SCLK_GPIO_PORT, USER_DEFINE_OLED_SCLK_GPIO_PIN)  
//时钟线命令SCLK线输出低电平
#define CLR_OLED_SCLK() GPIO_ResetBits(USER_DEFINE_OLED_SCLK_GPIO_PORT, USER_DEFINE_OLED_SCLK_GPIO_PIN)

//数据线SDIN线输出高电平
#define SET_OLED_SDIN() GPIO_SetBits(USER_DEFINE_OLED_SDIN_GPIO_PORT, USER_DEFINE_OLED_SDIN_GPIO_PIN)
//数据线SDIN线输出低电平
#define CLR_OLED_SDIN() GPIO_ResetBits(USER_DEFINE_OLED_SDIN_GPIO_PORT, USER_DEFINE_OLED_SDIN_GPIO_PIN)
  
/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/	 

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static  u8  s_iOLEDGRAM[128][8];                //OLED显存更新缓冲区
 
const u8 iconIndex[][2] = 
{
  {0, 0}, //FM
  {0, 7}, //音符
  {0, 14},//蓝牙
  {1, 0}, //电池空
  {1, 4}, //电池1格
  {1, 8}, //电池2格
  {1, 12},//电池3格
  {1, 16},//闹钟开
  {1, 20} //闹钟关
};

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/ 
static  void  ConfigOLEDGPIO(void);             //配置OLED的端口
static  void  ConfigOLEDReg(void);              //配置OLED的寄存器
static  void  OLEDWriteByte(u8 dat, u8 cmd);    //向OLED写入一个字节
static  u32   CalcPow(u8 m, u8 n);              //计算指定位置点数

//static  void  OLEDFill(u8 x1, u8 y1, u8 x2, u8 y2, u8 dot);       //填充指定区域

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: ConfigOLEDGPIO 
* 函数功能: 配置OLED的GPIO 
* 输入参数: void 
* 输出参数: void
* 返 回 值: void 
* 创建日期: 2018-03-01
* 注    意: CEPARK的OLED屏说明：VCC（3.3V）、CS、RES、DC、SCK、DIN、GND，其中RES需要与硬件RESET相连接       
*********************************************************************************************************/
static  void  ConfigOLEDGPIO(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;//定义结构体GPIO_InitStructure，用来配置OLED的GPIO
  
  RCC_APB2PeriphClockCmd(USER_DEFINE_OLED_CS_GPIO_CLK, ENABLE);        //使能OLED.CS的时钟
  RCC_APB2PeriphClockCmd(USER_DEFINE_OLED_RES_GPIO_CLK, ENABLE);       //使能OLED.RES的时钟
  RCC_APB2PeriphClockCmd(USER_DEFINE_OLED_DC_GPIO_CLK, ENABLE);        //使能OLED.DC的时钟
  RCC_APB2PeriphClockCmd(USER_DEFINE_OLED_SCLK_GPIO_CLK, ENABLE);      //使能OLED.SCLK的时钟
  RCC_APB2PeriphClockCmd(USER_DEFINE_OLED_SDIN_GPIO_CLK, ENABLE);      //使能OLED.SDIN的时钟
 
  //配置CS参数
  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_OLED_CS_GPIO_PIN;        //设置OLED.CS的引脚
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;                    //推挽输出模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                    //设置I/O口速率50MHz
  GPIO_Init(USER_DEFINE_OLED_CS_GPIO_PORT, &GPIO_InitStructure);       //根据参数初始化OLED.CS的GPIO端口
  GPIO_SetBits(USER_DEFINE_OLED_CS_GPIO_PORT, USER_DEFINE_OLED_CS_GPIO_PIN);//OLED.CS初始化为高电平
  
  //配置RSE参数
  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_OLED_RES_GPIO_PIN;       //设置OLED.RES的引脚
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;                    //推挽输出模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                    //设置I/O口速率50MHz
  GPIO_Init(USER_DEFINE_OLED_RES_GPIO_PORT, &GPIO_InitStructure);      //根据参数初始化OLED.RES的GPIO端口
  GPIO_SetBits(USER_DEFINE_OLED_RES_GPIO_PORT, USER_DEFINE_OLED_RES_GPIO_PIN);//OLED.RES初始化为高电平
  
  //配置DC参数
  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_OLED_DC_GPIO_PIN;        //设置OLED.DC的引脚
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;                    //推挽输出模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                    //设置I/O口速率50MHz
  GPIO_Init(USER_DEFINE_OLED_DC_GPIO_PORT, &GPIO_InitStructure);       //根据参数初始化OLED.DC的GPIO端口
  GPIO_SetBits(USER_DEFINE_OLED_DC_GPIO_PORT, USER_DEFINE_OLED_DC_GPIO_PIN);//OLED.DC初始化为高电平

  //配置SCLK参数                                                                 
  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_OLED_SCLK_GPIO_PIN;      //设置OLED.SCLK的引脚
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;                    //推挽输出模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                    //设置I/O口速率50MHz
  GPIO_Init(USER_DEFINE_OLED_SCLK_GPIO_PORT, &GPIO_InitStructure);     //根据参数初始化OLED.SCLK的GPIO端口
  GPIO_SetBits(USER_DEFINE_OLED_SCLK_GPIO_PORT, USER_DEFINE_OLED_SCLK_GPIO_PIN);//OLED.SCLK初始化为高电平
  
  //配置SDIN参数
  GPIO_InitStructure.GPIO_Pin   = USER_DEFINE_OLED_SDIN_GPIO_PIN;      //设置OLED.SDIN的引脚
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;                    //推挽输出模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                    //设置I/O口速率50MHz
  GPIO_Init(USER_DEFINE_OLED_SDIN_GPIO_PORT, &GPIO_InitStructure);     //根据参数初始化OLED.SDIN的GPIO端口
  GPIO_SetBits(USER_DEFINE_OLED_SDIN_GPIO_PORT, USER_DEFINE_OLED_SDIN_GPIO_PIN);//OLED.SDIN初始化为高电平
}

/*********************************************************************************************************
* 函数名称: ConfigOLEDReg 
* 函数功能: 配置OLED的寄存器 
* 输入参数: void 
* 输出参数: void
* 返 回 值: void 
* 创建日期: 2018-03-01
* 注    意: 
*********************************************************************************************************/
static  void  ConfigOLEDReg(void)
{
  OLEDWriteByte(0xAE, OLED_CMD); //关闭显示
  OLEDWriteByte(0xD5, OLED_CMD); //设置时钟分频因子,震荡频率
  OLEDWriteByte(80,   OLED_CMD); //[3:0],分频因子;[7:4],震荡频率
  OLEDWriteByte(0xA8, OLED_CMD); //设置驱动路数
  OLEDWriteByte(0X3F, OLED_CMD); //默认0X3F(1/64) 
  OLEDWriteByte(0xD3, OLED_CMD); //设置显示偏移
  OLEDWriteByte(0X00, OLED_CMD); //默认为0

  OLEDWriteByte(0x40, OLED_CMD); //设置显示开始行[5:0],行数.
  
  OLEDWriteByte(0x8D, OLED_CMD); //电荷泵设置
  OLEDWriteByte(0x14, OLED_CMD); //bit2，开启/关闭
  OLEDWriteByte(0x20, OLED_CMD); //设置内存地址模式
  OLEDWriteByte(0x02, OLED_CMD); //[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
  OLEDWriteByte(0xA1, OLED_CMD); //段重定义设置,bit0:0,0->0;1,0->127;
  OLEDWriteByte(0xC0, OLED_CMD); //设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
  OLEDWriteByte(0xDA, OLED_CMD); //设置COM硬件引脚配置
  OLEDWriteByte(0x12, OLED_CMD); //[5:4]配置
  
  OLEDWriteByte(0x81, OLED_CMD); //对比度设置
  OLEDWriteByte(0xEF, OLED_CMD); //1~255;默认0X7F (亮度设置,越大越亮)
  OLEDWriteByte(0xD9, OLED_CMD); //设置预充电周期
  OLEDWriteByte(0xf1, OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
  OLEDWriteByte(0xDB, OLED_CMD); //设置VCOMH 电压倍率
  OLEDWriteByte(0x30, OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

  OLEDWriteByte(0xA4, OLED_CMD); //全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
  OLEDWriteByte(0xA6, OLED_CMD); //设置显示方式;bit0:1,反相显示;0,正常显示	    						   
  OLEDWriteByte(0xAF, OLED_CMD); //开启显示	
}

/*********************************************************************************************************
* 函数名称: OLEDWriteByte
* 函数功能: 向OLED写入一个字节
* 输入参数: dat为要写入的数据或命令;cmd代表数据/命令标志，0-命令，1-数据
* 输出参数: void
* 返 回 值: void 
* 创建日期: 2018-03-01
* 注    意: 
*********************************************************************************************************/
static  void  OLEDWriteByte(u8 dat, u8 cmd)
{
  u8 i;                       //循环计数器
     
  //判断要写入数据还是写入命令
  if(OLED_CMD == cmd)         //标志cmd为传入命令时
  {                          
    CLR_OLED_DC();            //DC输出低电平用来读写命令  
  }                          
  else if(OLED_DATA == cmd)   //标志cmd为传入数据时
  {
    SET_OLED_DC();            //DC输出高电平用来读写数据  
  }                          
                             
  CLR_OLED_CS();              //CS输出低电平为写入数据或命令作准备
                             
  for(i = 0; i < 8; i++)      //循环八次，从高到低取出要写入的数据或命令的8个位
  {                          
    CLR_OLED_SCLK();          //SCLK输出低电平为写入数据作准备
                             
    if(dat & 0x80)            //判断要写入的数据或命令的最高位是“1”还是“0”
    {                        
      SET_OLED_SDIN();        //要写入的数据或命令的最高位是“1”，SDIN输出高电平表示“1”
    }                        
    else                     
    {                        
      CLR_OLED_SDIN();        //要写入的数据或命令的最高位是“0”，SDIN输出低电平表示“0”
    }                        
    SET_OLED_SCLK();          //SCLK输出高电平，SDIN的状态不再变化，此时写入数据线的数据
                             
    dat <<= 1;                //左移一位，次高位移到最高位
  }                          
                             
  SET_OLED_CS();              //OLED的CS输出高电平，不再写入数据或命令  
  SET_OLED_DC();              //OLED的DC输出高电平
} 

/*********************************************************************************************************
* 函数名称: OLEDFill
* 函数功能: 填充指定区域
* 输入参数: x1,y1,x2,y2 填充区域的对角坐标；dot:0,清空;1,填充	
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018-03-01
* 注    意: 确保x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63
*********************************************************************************************************/
//static  void  OLEDFill(u8 x1, u8 y1, u8 x2, u8 y2, u8 dot)  
//{  
//  u8 x;                               //当前横坐标
//  u8 y;                               //当前纵坐标
//  
//  for(x = x1; x <= x2; x++)           //设置填充的x轴范围
//  {
//    for(y = y1; y <= y2; y++)         //设置填充的y轴范围
//      { 
//        OLEDDrawPoint(x, y, dot);     //指定位置填充
//      }        
//  }
//  
//  OLEDRefreshGRAM();                  //更新显示
//}

/*********************************************************************************************************
* 函数名称: CalcPow
* 函数功能: m^n函数，求m的n次幂
* 输入参数: m，n 
* 输出参数: void 
* 返 回 值: void 
* 创建日期: 2018-03-01
* 注    意: 
*********************************************************************************************************/
static  u32 CalcPow(u8 m, u8 n)
{
  u32 result = 1;       //定义用来存放结果的变量             
   
  while(n--)            //随着每次循环，n递减，直至为0
  {
    result *= m;        //循环n次，相当于n个m相乘
  }
  
  return result;        //返回m的n次幂的值
}
 
/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/	
/*********************************************************************************************************
* 函数名称: InitOLED
* 函数功能: 初始化OLED
* 输入参数: void 
* 输出参数: void
* 返 回 值: void 
* 创建日期: 2018-03-01
* 注    意:
*********************************************************************************************************/   
void  InitOLED(void)
{ 
  ConfigOLEDGPIO();     //配置OLED的GPIO
  SET_OLED_RES();       //RES输出高电平
  ConfigOLEDReg();      //配置OLED的寄存器
  
  OLEDClear();          //清除OLED屏内容
}  

/*********************************************************************************************************
* 函数名称: OLEDDisplayOn
* 函数功能: 开启OLED显示
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018-03-01
* 注    意: 
*********************************************************************************************************/   
void  OLEDDisplayOn(void)
{
  OLEDWriteByte(0X8D, OLED_CMD);  //电荷泵设置
  OLEDWriteByte(0X14, OLED_CMD);  //bit2开启
  OLEDWriteByte(0XAF, OLED_CMD);  //开启显示
}

/*********************************************************************************************************
* 函数名称: OLEDDisplayOff
* 函数功能: 关闭OLED显示
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018-03-01
* 注    意: 
*********************************************************************************************************/ 
void  OLEDDisplayOff(void)
{
  OLEDWriteByte(0X8D, OLED_CMD);  //电荷泵设置
  OLEDWriteByte(0X10, OLED_CMD);  //向OLED寄存器写命令
  OLEDWriteByte(0XAE, OLED_CMD);  //关闭显示
}

/*********************************************************************************************************
* 函数名称: OLEDRefreshGRAM
* 函数功能: 更新OLED GRAM 
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018-03-01
* 注    意: 
*********************************************************************************************************/
void  OLEDRefreshGRAM(void)
{
  u8  i;                                          //循环计数器1
  u8  n;                                          //循环计数器2
            
  for(i = 0; i < 8; i++)                          //遍历每一页
  {           
    OLEDWriteByte(0xb0 + i, OLED_CMD);            //设置页地址（0~7）
    OLEDWriteByte(0x00, OLED_CMD);                //设置显示位置—列低地址
    OLEDWriteByte(0x10, OLED_CMD);                //设置显示位置—列高地址   
    for(n = 0; n < 128; n++)                      //遍历每一列
    {
      //通过循环将OLED显存更新缓冲区的数据更新OLED屏的显存上
      OLEDWriteByte(s_iOLEDGRAM[n][i], OLED_DATA);
    }    
  }   
}

/*********************************************************************************************************
* 函数名称: OLEDClear
* 函数功能: 清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018-03-01
* 注    意: 
*********************************************************************************************************/  
void  OLEDClear(void)  
{  
  u8  i;                            //循环计数器1
  u8  n;                            //循环计数器2
  
  for(i = 0; i < 8; i++)            //遍历每一页
  {
    for(n = 0; n < 128; n++)        //遍历每一列
    {
      s_iOLEDGRAM[n][i] = 0X00;   //将指定点清零
    }
  }

  OLEDRefreshGRAM();                //更新OLED GRAM
}

/*********************************************************************************************************
* 函数名称: OLEDDrawPoint
* 函数功能: 在指定位置画点
* 输入参数: x:0~127; y:0~63; fill:1-填充，0-清空
* 输出参数: void 
* 返 回 值: void
* 创建日期: 2018-03-01
* 注    意: （1）x-y轴体系的原点坐标在屏幕左上角；
*           （2）物理显示的y与SSD1306显存的COMn的n相加为63，当然，屏幕的列号（按字节）与SSD1306的列号（按
*                字节）相加为7。
*********************************************************************************************************/			   
void  OLEDDrawPoint(u8 x, u8 y, u8 fill)
{
  u8 pos;                           //存放点所在的页数
  u8 bx;                            //存放点所在的屏幕的行号
  u8 temp = 0;                      //用来存放画点位置相对于字节的位
  
  if(x > 127 || y > 63)             //如果指定位置超过额定范围
  {
    return;                         //返回空，函数结束
  }
  
  pos  = 7 - y / 8;                 //求指定位置所在页数
  bx   = y % 8;                     //求指定位置在上面求出页数中的行号
  temp = 1 << (7 - bx);             //（7 - bx）求出相应SSD1306的行号，并在字节中相应的位置为“1”

  if(fill)                          //判断填充标志为‘1’还是‘0’
  {
    s_iOLEDGRAM[x][pos] |= temp;    //如果填充标志为‘1’，指定点填充
  }
  else
  {
    s_iOLEDGRAM[x][pos] &= ~temp;	  //如果填充标志为‘0’，指定点清空
  }
}

/*********************************************************************************************************
* 函数名称: OLEDShow0Num
* 函数功能: 在OLED屏上指定位置显示带高位0的数字
* 输入参数: x,y :起点坐标;num:输入的数值;len :数字的位数;size:字体大小;
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018-03-01
* 注    意: num:数值(0~4294967295)
*********************************************************************************************************/	 	
void OLEDShow0Num(u8 x, u8 y, u32 num, u8 len, u8 size, u8 mode)
{
  u8 t;
  u8 temp;
  
  for(t = 0; t < len; t++)
  {
    temp = (num / CalcPow(10, len - t - 1)) % 10;
    
    OLEDShowChar(x + (size / 2) * t, y, temp + '0', size, mode);
  }
}
/*********************************************************************************************************
* 函数名称: OLEDShowNum
* 函数功能: 在OLED屏上指定位置显示数字
* 输入参数: x,y :起点坐标;num:输入的数值;len :数字的位数;size:字体大小;
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018-03-01
* 注    意: num:数值(0~4294967295)
*********************************************************************************************************/	 		  
void  OLEDShowNum(u8 x, u8 y, u32 num, u8 len, u8 size, u8 mode)
{
  u8  t;                                                     //循环计数器
  u8  temp;                                                  //用来存放要显示数字的各个位
  u8  enshow = 0;                                            //区分‘0’是否为高位0标志位
                                                          
  for(t = 0; t < len; t++)                               
  {                                                       
    temp = (num / CalcPow(10, len - t - 1)) % 10;            //按从高到低取出要显示数字的各个位，存到temp中      
    if(enshow == 0 && t < (len - 1))                         //如果标记enshow为‘0’并且还未取到最后一位
    {                                                     
      if(temp == 0 )                                         //如果temp等于‘0’
      {
        OLEDShowChar(x + (size / 2) * t, y, ' ', size, mode);   //此时的‘0’在高位，用空格替代
        continue;                                            //提前结束本次循环，进入下一次循环
      }
      else
      {
        enshow = 1;                                          //否则将标记enshow置为‘1’
      }     
    }
    OLEDShowChar(x + (size / 2) * t, y, temp + '0', size, mode);//在指定位置显示得到的数字
  }
}

/*********************************************************************************************************
* 函数名称: OLEDShowChar
* 函数功能: 在OLED屏上指定位置显示字符
* 输入参数: x:0~127;y:0~63;size:选择字体 16/12;mode:0,反白显示;1,正常显示
            ptn:选择取模方式；chr为指定显示的字符
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018-03-01
* 注    意: 
*********************************************************************************************************/
void  OLEDShowChar(u8 x, u8 y, u8 chr, u8 size, u8 mode)
{ 
  u8  temp;                           //用来存放字符顺向逐列式的相对位置
  u8  t;                              //循环计数器1
  u8  t1;                             //循环计数器2
  u8  y0 = y;                         //当前操作的行数

  chr = chr - ' ';                    //得到相对于‘ ’偏移的值，从而求出要打印字符在asc1中的行

  for(t = 0; t < size; t++)           //循环逐列显示
  {   
    if(size == 12)                    //判断字号大小，选择相对的顺向逐列式
    {
      temp = asc1_1206[chr][t];       //取出字符在asc1_1206中的第t列
    }
    else
    {
      temp = asc1_1608[chr][t];       //取出字符在asc1_1608中的第t列
    }      
  
    for(t1 = 0; t1 < 8; t1++)         //#在一个字符的第t列的横向范围（8个像素）内显示点
    {
      if(temp & 0x80)                 //取出temp的最高位，并判断为‘0’还是‘1’
      {
        OLEDDrawPoint(x, y, mode);    //如果temp的最高位为‘1’填充指定位置的点
      }
      else
      {
        OLEDDrawPoint(x, y, !mode);   //如果temp的最高位为‘0’清除指定位置的点
      }

      temp <<= 1;                     //左移一位，次高位移到最高位
      y++;                            //进入下一行

      if((y - y0) == size)            //如果显示完一列
      {
        y = y0;                       //行号回到原来的位置
        x++;                          //进入下一列
        break;                        //跳出上面带#的循环
      }
    } 
	}          
}

/*********************************************************************************************************
* 函数名称: OLEDShowString
* 函数功能: 在OLED屏上指定位置显示字符串
* 输入参数: x,y:起点坐标;*p:字符串起始地址;size:字号大小
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018-03-01
* 注    意: 
*********************************************************************************************************/
void  OLEDShowString(u8 x, u8 y, const u8* p, u8 size, u8 mode)
{
#define MAX_CHAR_POSX 122         //OLED屏幕横向的最大范围
#define MAX_CHAR_POSY 58          //OLED屏幕纵向的最大范围
          
  while(*p != '\0')               //指针不等于结束符时，循环进入
  {       
    if(x > MAX_CHAR_POSX)         //如果x超出指定最大范围，x赋值为0
    {
      x  = 0;                     
      y += 16;                    //显示到下一行左端
    }

    if(y > MAX_CHAR_POSY)         //如果y超出指定最大范围，x和y均赋值为0
    {
      OLEDClear();                //清除OLED屏幕内容
      y = x = 0;                  //显示到OLED屏幕左上角
    }

    OLEDShowChar(x, y, *p, size, mode);//指定位置显示一个字符
        
    x += size / 2;                  //一个字符横向占8个像素点
    p++;                          //指针指向下一个字符
  }  
}

/*********************************************************************************************************
* 函数名称: OLEDClearArea
* 函数功能: 清除屏幕上指定的区域
* 输入参数: x1,y1:起点坐标;x2,y2:终点坐标;
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018-03-01
* 注    意: 
*********************************************************************************************************/
void OLEDClearArea(u8 x1, u8 y1, u8 x2, u8 y2)
{
  u8 xx, yy;
  
  for(yy = y1; yy <= y2; yy++)
  {
    for(xx = x1; xx <= x2; xx++)
    {
      OLEDDrawPoint(xx, yy, 0);
    }
  }
}

/*********************************************************************************************************
* 函数名称: OLEDShowIcon
* 函数功能: 在指定区域显示图标
* 输入参数: x1,y1:起点坐标;x2,y2:终点坐标;
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018-03-01
* 注    意: 
*********************************************************************************************************/
void OLEDShowIcon(u8 x, u8 y, u8 object, u8 mode)
{
  u8  temp;                           //用来存放字符顺向逐列式的相对位置
  u8  t;                              //循环计数器1
  u8  t1;                             //循环计数器2
  u8  y0 = y;                         //当前操作的行数
  u8  iconType;
  u8  index;
  u8  rowSize;
  u8  colSize;
  
  iconType = iconIndex[object][0];
  index    = iconIndex[object][1];
  
  if(iconType)
  {
    rowSize = 20 * 2;
    colSize = 16;
  }
  else
  {
    rowSize = 42 * 3;
    colSize = 24;
  }
  
  for(t = 0; t < rowSize; t++)           //循环逐列显示
  {   
    if(iconType)                    //判断字号大小，选择相对的顺向逐列式
    {
      temp = statusIcon[index][t];       //取出字符在asc1_1206中的第t列
    }
    else
    {
      temp = deskIcon[index][t];       //取出字符在asc1_1608中的第t列
    }      
  
    for(t1 = 0; t1 < 8; t1++)         //#在一个字符的第t列的横向范围（8个像素）内显示点
    {
      if(temp & 0x80)                 //取出temp的最高位，并判断为‘0’还是‘1’
      {
        OLEDDrawPoint(x, y, mode);    //如果temp的最高位为‘1’填充指定位置的点
      }
      else
      {
        OLEDDrawPoint(x, y, !mode);   //如果temp的最高位为‘0’清除指定位置的点
      }

      temp <<= 1;                     //左移一位，次高位移到最高位
      y++;                            //进入下一行

      if((y - y0) == colSize)            //如果显示完一列
      {
        y = y0;                       //行号回到原来的位置
        x++;                          //进入下一列
        break;                        //跳出上面带#的循环
      }
    } 
	}    
}

/*********************************************************************************************************
* 函数名称: OLEDShowAlarmRing
* 函数功能: OLED显示闹钟响了
* 输入参数: x1,y1:起点坐标;x2,y2:终点坐标;
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018-03-01
* 注    意: 
*********************************************************************************************************/
void OLEDShowAlarmRing(void)
{
  u8  temp;                           //用来存放字符顺向逐列式的相对位置
  u8  t;                              //循环计数器1
  u8  t1;                             //循环计数器2
  u8  y0 = 16;                         //当前操作的行数
  u8  y = y0;
  u8  x = 48;
  
  for(t = 0; t < 128; t++)           //循环逐列显示
  {   
    temp = AlarmRingIcon[0][t];       //取出字符在asc1_1206中的第t列     
  
    for(t1 = 0; t1 < 8; t1++)         //#在一个字符的第t列的横向范围（8个像素）内显示点
    {
      if(temp & 0x80)                 //取出temp的最高位，并判断为‘0’还是‘1’
      {
        OLEDDrawPoint(x, y, 1);    //如果temp的最高位为‘1’填充指定位置的点
      }
      else
      {
        OLEDDrawPoint(x, y, 0);   //如果temp的最高位为‘0’清除指定位置的点
      }

      temp <<= 1;                     //左移一位，次高位移到最高位
      y++;                            //进入下一行

      if((y - y0) == 32)            //如果显示完一列
      {
        y = y0;                       //行号回到原来的位置
        x++;                          //进入下一列
        break;                        //跳出上面带#的循环
      }
    } 
	}  
}
