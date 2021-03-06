﻿/*********************************************************************************************************
* 模块名称: ExFsFun.h
* 摘    要: 
* 当前版本: 1.0.0
* 作    者: 
* 完成日期: 
* 内    容:
* 注    意: none                                                                  
**********************************************************************************************************
* 取代版本: 
* 作    者:
* 完成日期: 
* 修改内容:
* 修改文件: 
*********************************************************************************************************/
#ifndef _EX_FS_FUN_H_
#define _EX_FS_FUN_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include <stm32f10x.h>
#include "DataType.h"
#include "ff.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
//f_typetell返回的类型定义
//根据表FILE_TYPE_TBL获得.在exfuns.c里面定义
#define T_BIN		0X00	  //bin文件
#define T_LRC		0X10	  //lrc文件
#define T_NES		0X20	  //nes文件
#define T_TEXT	0X30	  //.txt文件
#define T_C			0X31	  //.c文件
#define T_H			0X32    //.h文件
#define T_MP1		0X40	  //MP1文件
#define T_MP2		0X41	  //MP2文件 
#define T_MP3		0X42	  //MP3文件
#define T_MP4		0X43	  //MP4文件
#define T_M4A		0X44	  //M4A文件
#define T_3GP		0X45	  //3GP文件
#define T_3G2		0X46	  //3G2文件
#define T_OGG		0X47	  //OGG文件
#define T_AAC		0X48	  //AAC文件
#define T_WMA		0X49	  //WMA文件
#define T_WAV		0X4A	  //WAV文件
#define T_MID		0X4B	  //MIDI文件
#define T_FLAC	0X4C	  //FLAC文件 
#define T_BMP		0X50	  //bmp文件
#define T_JPG		0X51	  //jpg文件
#define T_JPEG	0X52	  //jpeg文件		 
#define T_GIF		0X53	  //gif文件   
#define T_AVI		0X60	  //avi文件  

extern FATFS *fs[_VOLUMES];
extern FIL   *fnew;	
extern FIL   *ftemp;
extern u8    *fatbuf;
extern DIR   dir;

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/
void InitFatFs(void);                           //初始化文件系统
void TestFatFs(void);                           //测试文件系统
u8   InitExFsFun(void);							            //申请内存
u8   f_typetell(u8 *fname);						          //识别文件类型
u8   exf_getfree(u8 *drv,u32 *total,u32 *free);	//得到磁盘总容量和剩余容量 

#endif


