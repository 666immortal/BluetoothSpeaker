 /*********************************************************************************************************
* 模块名称: ADC.c
* 摘    要: 
* 当前版本: 1.0.0
* 作    者: 
* 完成日期: 2018年03月01日
* 内    容:
* 注    意: none                                                                  
**********************************************************************************************************
* 取代版本: 1.0.0
* 作    者:
* 完成日期: 
* 修改内容: 
* 修改文件: 
*********************************************************************************************************/
/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "ADC.h"
#include "SysTick.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/ 

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static u16 GetADC(void);    //获得ADC采样值

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: GetADC
* 函数功能: 获得ADC采样值
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/
static u16 GetADC(void)   
{
  //设置ADC,ADC通道,通道数目，采样时间	
	ADC_RegularChannelConfig(USER_DEFINE_ADC, USER_DEFINE_ADC_CHANNEL, 1, ADC_SampleTime_239Cycles5 );	  			    
  
	ADC_SoftwareStartConvCmd(USER_DEFINE_ADC, ENABLE);		     //使能指定的ADC的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(USER_DEFINE_ADC, ADC_FLAG_EOC));  //等待转换结束

	return ADC_GetConversionValue(USER_DEFINE_ADC);	           //返回最近一次ADC规则组的转换结果
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitADC
* 函数功能: 初始化ADC
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 这里我们仅以规则通道为例
*********************************************************************************************************/													   
void InitADC(void)
{ 	
	ADC_InitTypeDef   ADC_InitStructure;      //定义结构体ADC_InitStructure,用来配置ADC
	GPIO_InitTypeDef  GPIO_InitStructure;     //定义结构体GPIO_InitStructure，用来配置ADC通道的GPIO

	RCC_APB2PeriphClockCmd(USER_DEFINE_ADC_CLK, ENABLE);	      //使能ADC的时钟
  RCC_APB2PeriphClockCmd(USER_DEFINE_ADC_GPIO_CLK, ENABLE);   //使能ADC通道的GPIO端口时钟
 
	RCC_ADCCLKConfig(USER_DEFINE_ADC_CLK_DIVE);  //设置ADC分频因子为6，时钟为72M/6=12M,ADC最大时间不能超过14M
                   
	GPIO_InitStructure.GPIO_Pin = USER_DEFINE_ADC_GPIO_PIN;     //设置模拟通道输入引脚      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		            //模拟输入模式
	GPIO_Init(USER_DEFINE_ADC_GPIO, &GPIO_InitStructure);	      //根据参数初始化ADC通道的GPIO端口
                                                                 
	ADC_DeInit(USER_DEFINE_ADC);                            //复位ADC 
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	    //ADC工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	          //模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;     //模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;   //转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  //ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;                 //顺序进行规则转换的ADC通道的数目
	ADC_Init(USER_DEFINE_ADC, &ADC_InitStructure);          //根据ADC_InitStruct中指定的参数初始化ADC   

	ADC_Cmd(USER_DEFINE_ADC, ENABLE);	                      //使能ADC
	
	ADC_ResetCalibration(USER_DEFINE_ADC);	                //使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(USER_DEFINE_ADC));	//等待复位校准结束
	
	ADC_StartCalibration(USER_DEFINE_ADC);	                //开启AD校准
 
	while(ADC_GetCalibrationStatus(USER_DEFINE_ADC));	      //等待校准结束
 
}				

/*********************************************************************************************************
* 函数名称: GetADCAverage
* 函数功能: 计算ADC采样数据的平均值
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/
u16 GetADCAverage(u8 sampletimes)
{
	u32 tempVal = 0;                                  //定义一个变量用来表示采样数据之和
	u8 t;                                             //定义一个变量执行循环语句
  
	for(t = 0; t < sampletimes; t++)                  //循环采样加和
	{
		tempVal += GetADC();                            //对采样数据进行加和并赋给tempVal
                                              
		DelayNms(5);                                    //延时5ms
	}
  
	return tempVal / sampletimes;                     //返回采样数据的平均值
} 	 
