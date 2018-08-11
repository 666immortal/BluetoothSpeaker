/*********************************************************************************************************
* 模块名称: ExFsFun.c
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

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "ExFsFun.h"
#include "Malloc.h"
#include "UART.h"
#include <string.h>

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define FILE_MAX_TYPE_NUM		6	  //最多FILE_MAX_TYPE_NUM个大类
#define FILE_MAX_SUBT_NUM		13	//最多FILE_MAX_SUBT_NUM个小类

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/
u8*const FILE_TYPE_TBL[FILE_MAX_TYPE_NUM][FILE_MAX_SUBT_NUM]=
{
{"BIN"},			//BIN文件
{"LRC"},			//LRC文件
{"NES"},			//NES文件
{"TXT","C","H"},	//文本文件
{"MP1","MP2","MP3","MP4","M4A","3GP","3G2","OGG","AAC","WMA","WAV","MID","FLAC"},//音乐文件
{"BMP","JPG","JPEG","GIF"},//图片文件 
};

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
FATFS *fs[_VOLUMES];                                          //逻辑磁盘工作区.	 
FIL   *fnew;	  		                                          //文件1
FIL   *ftemp;	  		                                          //文件2.
FILINFO fileinfo;	                                            //文件信息
DIR   dir;  			                                            //目录
u8    *fatbuf;			                                          //SD卡数据缓存区
FRESULT res_sd;                                               //文件操作结果
UINT  fnum;                                                   //文件成功读写数量
BYTE ReadBuffer[1024] = {0};                                  //读缓冲区
BYTE WriteBuffer1[] = "Test SDcard File System，FatFs!\r\n";  //写缓冲区1
BYTE WriteBuffer2[] = "Test Flash_File System，FatFs!\r\n";   //写缓冲区1  

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static u8 char_upper(u8 c);

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: char_upper
* 函数功能: 将小写字母转为大写字母,如果是数字,则保持不变.
* 输入参数: void 
* 输出参数: void
* 返 回 值: 转化后的大写字母
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
static u8 char_upper(u8 c)
{
	if(c < 'A')
  {
    return c;       //数字,保持不变.
  }
  
	if(c >= 'a')
  {
    return c-0x20;  //变为大写.
  }  
	else 
  {
    return c;       //大写,保持不变
  }
}	 

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitFatFs
* 函数功能: 初始化文件系统
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  InitFatFs(void)
{
  printf(">>>This is FatFs test lab\r\n");
  //在外部SD卡挂载文件系统，文件系统挂载会对SD卡初始化
  printf("Now start to mount SD_FatFs...\r\n");
  res_sd = f_mount(fs[0], "0:", 1);
  //如果没有文件系统就格式化创建新的文件系统
  if(res_sd == FR_NO_FILESYSTEM)
  {
    printf("SDcard FatFs has not been done! Now rebulit SDcard FatFs!\r\n");
    //创建文件系统
    res_sd = f_mkfs("0:", 1, 4096);
    
    if(res_sd == FR_OK)
    {
      printf("Init SD_FatFs success!...\r\n");
      //格式化后取消挂载
      res_sd = f_mount(NULL, "0:", 1);
      //重新挂载
      res_sd = f_mount(fs[0], "0:", 1);
    }
    else
    {
      printf("FAILED!!...\r\n");
      while(1)
      {
        
      }
    }
  }
  else if(res_sd != FR_OK)
  {
    printf("Mount SD_FatFs Failed!..(%d)\r\n", res_sd);
    printf("Maybe SDcard Init failed!...\r\n");
    while(1)
    {
      
    }
  }
  else
  {
    printf(">>>Mount SD_FatFs success!\r\n");
  }
/*********************************************************************
*                              分割线
*********************************************************************/
  printf("Now start to mount Flash_FatFs...\r\n");
  //在外部Flash挂载文件系统，文件系统挂载会对W25Qxx初始化
  res_sd = f_mount(fs[1], "1:", 1);
  //如果没有文件系统就格式化创建新的文件系统
  if(res_sd == FR_NO_FILESYSTEM)
  {
    printf("Flash_Flash has not been done! Now rebulit Flash_FatFs!\r\n");
    //创建文件系统
    res_sd = f_mkfs("1:", 1, 4096);
    
    if(res_sd == FR_OK)
    {
      printf("Init Flash_FatFs success!...\r\n");
      //格式化后取消挂载
      res_sd = f_mount(NULL, "1:", 1);
      //重新挂载
      res_sd = f_mount(fs[1], "1:", 1);
    }
    else
    {
      printf("FAILED!!...\r\n");
      while(1)
      {
        
      }
    }
  }
  else if(res_sd != FR_OK)
  {
    printf("Mount Flash_FatFs Failed!..(%d)\r\n", res_sd);
    printf("Maybe W25Qxx Init failed!...\r\n");
    while(1)
    {
      
    }
  }
  else
  {
    printf(">>>Mount Flash_FatFs success!\r\n");
  }
}


/*********************************************************************************************************
* 函数名称: TestFatFs
* 函数功能: 测试文件系统
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void  TestFatFs(void)
{
  printf("\r\n>>>Now start to test SDcard FatFs!..\r\n");
  
  //文件系统写测试
  //打开文件，如果文件不存在，就创建它
  res_sd = f_open(fnew, "0:test3.txt", FA_WRITE | FA_CREATE_ALWAYS);
  if(res_sd == FR_OK)
  {
    printf("Creat and open a new file <Test3.txt> successful!\r\n");
    //将指定储存区内容写入到文件内
    res_sd = f_write(fnew, WriteBuffer1, sizeof(WriteBuffer1), &fnum);
    if(res_sd == FR_OK)
    {
      printf("Write file <Test3.txt> successful!\r\n");
      printf("Write %d bytes!\r\n", fnum);
      printf("The detail I write in:\r\n%s\r\n", WriteBuffer1);
    }
    else
    {
      printf("Write failed!\r\n");
    }
    //不再读写，关闭文件
    f_close(fnew);
  }
  else
  {
    printf("Creat or open the file failed!(%d)\r\n", res_sd);
  }
  
  //文件系统读测试
  printf(">>>Now start to test read operation!..\r\n");
  
  res_sd = f_open(fnew, "0:Test1.txt", FA_OPEN_EXISTING | FA_READ);
  if(res_sd == FR_OK)
  {
    printf("Open file successful!..\r\n");
    //将文件的内容读取到读缓冲区
    res_sd = f_read(fnew, ReadBuffer, sizeof(ReadBuffer), &fnum);
    
    if(res_sd == FR_OK)
    {
      printf("Read file successful!\r\n");
      printf("Read %d bytes!\r\n", fnum);
      printf("The detail I read is\r\n%s\r\n", ReadBuffer);
    }
    else
    {
      printf("Read file failed!..\r\n");
    }
  }
  else
  {
    printf("Open file failed!..\r\n");
  }
  //不再读写，关闭文件
  f_close(fnew);
  
  //不再使用文件系统，取消挂载文件系统
  //f_mount(1, NULL);
  
  printf(">>>Test1 Over!..\r\n");
  
/*********************************************************************
*                              分割线
*********************************************************************/
  printf("\r\n>>>Now start to test Flash_FatFs!..\r\n");
  
  //文件系统写测试
  //打开文件，如果文件不存在，就创建它
  res_sd = f_open(fnew, "1:test4.txt", FA_WRITE | FA_CREATE_ALWAYS);
  if(res_sd == FR_OK)
  {
    printf("Creat and open a new file <Test4.txt> successful!\r\n");
    //将指定储存区内容写入到文件内
    res_sd = f_write(fnew, WriteBuffer2, sizeof(WriteBuffer2), &fnum);
    if(res_sd == FR_OK)
    {
      printf("Write file <Test4.txt> successful!\r\n");
      printf("Write %d bytes!\r\n", fnum);
      printf("The detail I write in:\r\n%s\r\n", WriteBuffer2);
    }
    else
    {
      printf("Write failed!\r\n");
    }
    //不再读写，关闭文件
    f_close(fnew);
  }
  else
  {
    printf("Creat or open the file failed!(%d)\r\n", res_sd);
  }
  
  //文件系统读测试
  printf(">>>Now start to test read operation!..\r\n");
  
  res_sd = f_open(fnew, "1:Test4.txt", FA_OPEN_EXISTING | FA_READ);
  if(res_sd == FR_OK)
  {
    printf("Open file successful!..\r\n");
    //将文件的内容读取到读缓冲区
    res_sd = f_read(fnew, ReadBuffer, sizeof(ReadBuffer), &fnum);
    
    if(res_sd == FR_OK)
    {
      printf("Read file successful!\r\n");
      printf("Read %d bytes!\r\n", fnum);
      printf("The detail I read is\r\n%s\r\n", ReadBuffer);
    }
    else
    {
      printf("Read file failed!..\r\n");
    }
  }
  else
  {
    printf("Open file failed!..\r\n");
  }
  //不再读写，关闭文件
  f_close(fnew);
  
  //不再使用文件系统，取消挂载文件系统
  //f_mount(0, NULL);
  
  printf(">>>Test2 Over!..\r\n");
}

/*********************************************************************************************************
* 函数名称: InitExFsFun
* 函数功能: 初始化处理文件系统函数，为文件系统变量申请内存
* 输入参数: void 
* 输出参数: void
* 返 回 值: 0,成功；1,失败
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8 InitExFsFun(void)
{
  u8 i;
  
	for(i = 0; i < _VOLUMES; i++)
	{
		fs[i] = (FATFS*)MallocMemExt(USER_DEFINE_SRAMIN, sizeof(FATFS));	//为磁盘i工作区申请内存	
		if(!fs[i])
    {
      break;
    }
	}
  
	fnew = (FIL*)MallocMemExt(USER_DEFINE_SRAMIN, sizeof(FIL));		  //为file申请内存
	ftemp = (FIL*)MallocMemExt(USER_DEFINE_SRAMIN, sizeof(FIL));		//为ftemp申请内存
	fatbuf = (u8*)MallocMemExt(USER_DEFINE_SRAMIN, 512);				    //为fatbuf申请内存
  
	if(i == _VOLUMES && fnew && ftemp && fatbuf)
  {
    return 0;  //申请有一个失败,即失败.
  }
	else 
  {
    return 1;	
  }
}

/*********************************************************************************************************
* 函数名称: f_typetell
* 函数功能: 报告文件的类型
* 输入参数: fname:文件名
* 输出参数: void
* 返 回 值: 0XFF,表示无法识别的文件类型编号.其他,高四位表示所属大类,低四位表示所属小类.
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8 f_typetell(u8 *fname)
{
	u8 tbuf[5];
	u8 *attr = '\0';//后缀名
	u8 i = 0, j;
  
	while(i < 250)
	{
		i++;
    
		if(*fname == '\0')
    {
      break;//偏移到了最后了.
    }
    
		fname++;
	}
  
	if(i == 250)
  {
    return 0XFF;//错误的字符串.
  }
  
 	for(i = 0; i < 5; i++)//得到后缀名
	{
		fname--;
    
		if(*fname == '.')
		{
			fname++;
			attr = fname;
      
			break;
		}
    
  }
  
	strcpy((char *)tbuf, (const char*)attr);//copy
  
 	for(i = 0; i < 4; i++)
  {
    tbuf[i] = char_upper(tbuf[i]);//全部变为大写 
  }
  
	for(i = 0; i < FILE_MAX_TYPE_NUM; i++)	//大类对比
	{
		for(j = 0; j < FILE_MAX_SUBT_NUM; j++)//子类对比
		{
			if(*FILE_TYPE_TBL[i][j] == 0)
      {
        break;//此组已经没有可对比的成员了.        
      }
      
			if(strcmp((const char *)FILE_TYPE_TBL[i][j], (const char *)tbuf) == 0)//找到了
			{
				return (i << 4) | j;
			}
		}
	}
  
	return 0XFF;//没找到		 			   
}	 

/*********************************************************************************************************
* 函数名称: exf_getfree
* 函数功能: 得到磁盘剩余容量
* 输入参数: drv:磁盘编号("0:"/"1:")；total:总容量，单位KB；free:剩余容量，单位KB
* 输出参数: void
* 返 回 值: 0,正常.其他,错误代码
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8 exf_getfree(u8 *drv, u32 *total, u32 *free)
{
	FATFS *fs1;
	u8 res;
  u32 fre_clust = 0, fre_sect = 0, tot_sect = 0;
  
  //得到磁盘信息及空闲簇数量
  res = (u32)f_getfree((const TCHAR*)drv, (DWORD*)&fre_clust, &fs1);
  
  if(res == 0)
	{											   
	    tot_sect = (fs1->n_fatent - 2) * fs1->csize;	//得到总扇区数
	    fre_sect = fre_clust * fs1->csize;			      //得到空闲扇区数	   
#if _MAX_SS != 512				  				                //扇区大小不是512字节,则转换为512字节
		tot_sect *= fs1->ssize / 512;
		fre_sect *= fs1->ssize / 512;
#endif	  
		*total = tot_sect >> 1;   	                    //单位为KB
		*free = fre_sect >> 1;	                        //单位为KB 
 	}
  
	return res;
}	
