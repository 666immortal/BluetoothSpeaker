﻿/*********************************************************************************************************
* 模块名称: Text.c
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
#include "SysIT.h" 
#include "FontUpd.h"
#include "W25Qxx.h"
#include "Text.h"	
#include <string.h>
#include "UART.h"						
#include "OLED.h"
 	 
/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              结构体枚举定义
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
* 函数名称: Get_HzMat
* 函数功能: 从字库中查找出字模
* 输入参数: code 字符串的开始地址,GBK码；mat数据存放地址(size/8+((size%8)?1:0))*(size) bytes大小；size:字体大小
* 输出参数: void
* 返 回 值: void 
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void Get_HzMat(unsigned char *code, unsigned char *mat, u8 size)
{		    
	unsigned char qh, ql;
	unsigned char i;					  
	unsigned long foffset; 
  
	u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size);//得到字体一个字符对应点阵集所占的字节数	 
	qh = *code;
	ql = *(++code);
  
	if(qh < 0x81 || ql < 0x40 || ql == 0xff || qh == 0xff)//非 常用汉字
	{   		    
	  for(i = 0; i < csize; i++)
    {
      *mat++ = 0x00;//填充满格
    }
    
	  return;     //结束访问
	}  
  
	if(ql < 0x7f)
  {
    ql -= 0x40;//注意!
  }
	else 
  {
    ql -= 0x41;
  }
  
	qh -= 0x81;   
	foffset = ((unsigned long)190 * qh + ql) * csize;	//得到字库中的字节偏移量  	
  
	switch(size)
	{
		case 12:
			W25QxxRead(mat, foffset+ftinfo.f12addr, csize);
    
			break;
		case 16:
			W25QxxRead(mat, foffset+ftinfo.f16addr, csize);
    
			break;
		case 24:
			W25QxxRead(mat, foffset+ftinfo.f24addr, csize);
    
			break;
			
	}     												    
} 

/*********************************************************************************************************
* 函数名称: Show_Font
* 函数功能: 显示一个指定大小的汉字
* 输入参数: x,y :汉字的坐标；font:汉字GBK码；size:字体大小；mode:0,正常显示,1,叠加显示
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/   
void Show_Font(u16 x, u16 y, u8 *font, u8 size, u8 mode)
{
	u8 temp, t, t1;
	u16 y0 = y;
	u8 dzk[72];   
	u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size);//得到字体一个字符对应点阵集所占的字节数	 
  
	if(size != 12 && size != 16 && size != 24)
  {
    return;	//不支持的size
  }
  
	Get_HzMat(font, dzk, size);	//得到相应大小的点阵数据 
  
	for(t = 0; t < csize; t++)
	{   												   
		temp = dzk[t];			//得到点阵数据       
    
		for(t1 = 0; t1 < 8; t1++)
		{
			if(temp & 0x80)
      {
        OLEDDrawPoint(x, y, mode);    //如果temp的最高位为‘1’填充指定位置的点
      }
			else
      {
        OLEDDrawPoint(x, y, !mode);   //如果temp的最高位为‘0’清除指定位置的点
      }
      
			temp <<= 1;
			y++;
      
			if((y - y0) == size)
			{
				y = y0;
				x++;
				break;
			}
		}  	 
	}  
}

/*********************************************************************************************************
* 函数名称: Show_Str
* 函数功能: 在指定位置开始显示一个字符串	，支持自动换行
* 输入参数: (x,y):起始坐标，width,height:区域，str  :字符串，size :字体大小，mode:0,非叠加方式;1,叠加方式 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/   	   		   
void Show_Str(u16 x, u16 y, u16 width, u16 height, u8*str, u8 size, u8 mode)
{					
	u16 x0 = x;
	u16 y0 = y;							  	  
  u8 bHz = 0;     //字符或者中文 
  
  while(*str != 0)//数据未结束
  { 
    if(!bHz)
    {
	    if(*str > 0x80)
      {
        bHz = 1;//中文 
      }
	    else              //字符
	    {      
        if(x > (x0 + width - size / 2))//换行
        {				   
          y += size;
          x = x0;	   
        }							    
	      if(y > (y0 + height - size))
        {
          break;//越界返回      
        }
        
	      if(*str == 13)//换行符号
	      {         
	         y+=size;
           x=x0;
	         str++; 
	      }  
	      else 
        {
           OLEDShowChar(x, y, *str, size, mode);//有效部分写入 
        }
        str++; 
	      x += size / 2; //字符,为全字的一半 
	    }
    }
    else//中文 
    {     
        bHz = 0;//有汉字库    
        if(x > (x0 + width - size))//换行
        {	    
          y += size;
          x = x0;		  
        }
	      if(y > (y0 + height - size))
        {
          break;//越界返回  						     
        }
        
	      Show_Font(x, y, str, size, mode); //显示这个汉字,空心显示 
	      str += 2; 
	      x += size;//下一个汉字偏移	    
      }						 
  }   
}  

/*********************************************************************************************************
* 函数名称: Show_Str_Mid
* 函数功能: 在指定宽度的中间显示字符串，如果字符长度超过了len,则用Show_Str显示
* 输入参数: len:指定要显示的宽度		
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/ 		  
void Show_Str_Mid(u16 x, u16 y, u8* str, u8 size, u8 len)
{
	u16 strlenth = 0;
  
  strlenth = strlen((const char*)str);
	strlenth *= size / 2;
  
	if(strlenth > len)
  {
    Show_Str(x, y, 128, 64, str, size, 1);
  }
	else
	{
		strlenth = (len - strlenth) / 2;
	  Show_Str(strlenth + x, y, 128, 64, str, size, 1);
	}
}   
