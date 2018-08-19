/*********************************************************************************************************
* 模块名称: MusicPlayer.c
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
#include "MusicPlayer.h"
#include "VS10xx.h"   
#include "SysTick.h"
#include "LED.h"
#include "KeyOne.h"
#include "ProcKeyOne.h"
#include "OLED.h"     
#include "Malloc.h"
#include "Text.h"
#include <string.h>
#include "ExFsFun.h"  
#include "ff.h"   
#include "flac.h"  
#include "UART.h"  
#include "UI.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
u16 f_kbps = 0;//歌曲文件位率
u8  musicKey = 0;
u8  Playmusic = 0;

extern u8 NowInterFace;
extern u8 NowPointAt;
extern u8 AlarmStop;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: mp3_index_show
* 函数功能: 显示曲目索引
* 输入参数: index:当前索引；total:总文件数
* 输出参数: void
* 返 回 值: void
* 创建日期: 20XX年XX月XX日
* 注    意: 
*********************************************************************************************************/
void mp3_index_show(u16 index,u16 total)
{
  //显示当前曲目的索引,及总曲目数
  OLEDShow0Num(0, 48, index, 3, 12, 1);
  OLEDShowChar(18, 48, '/', 12, 1);
  OLEDShow0Num(24, 48, total, 3, 12, 1);
  OLEDRefreshGRAM();
//  LCD_ShowxNum(30 + 0, 230, index, 3, 16, 0X80);    //索引
//  LCD_ShowChar(30 + 24, 230, '/', 16, 0);
//  LCD_ShowxNum(30 + 32, 230, total, 3, 16, 0X80);   //总曲目              
}

/*********************************************************************************************************
* 函数名称: mp3_vol_show
* 函数功能: 显示当前音量
* 输入参数: vol,音量
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void mp3_vol_show(u8 vol)
{  
  OLEDShowString(54, 48, "VOL:", 12, 1);
  OLEDShowNum(84, 48, vol, 2, 12, 1);
  OLEDRefreshGRAM();
  
//  LCD_ShowString(30 + 110, 230, 200, 16, 16, "VOL:");        
//  LCD_ShowxNum(30 + 142, 230, vol, 2, 16, 0X80);   //显示音量   
}

/*********************************************************************************************************
* 函数名称: mp3_msg_show
* 函数功能: 显示播放时间,比特率 信息
* 输入参数: lenth:歌曲总长度
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/  
void mp3_msg_show(u32 lenth)
{  
  static u16 playtime = 0;//播放时间标记       
  u16 time = 0;// 时间变量
  u16 temp = 0;  
  
  if(f_kbps == 0xffff)//未更新过
  {
    playtime = 0;
    f_kbps = VS_Get_HeadInfo();     //获得比特率
  }     
  
  time = VS_Get_DecodeTime(); //得到解码时间
  
  if(playtime == 0)
  {
    playtime = time;
  }
  else if((time != playtime) && (time != 0))//1s时间到,更新显示数据
  {
    playtime = time;//更新时间                
    temp = VS_Get_HeadInfo(); //获得比特率              
    if(temp != f_kbps)
    {
      f_kbps = temp;//更新KBPS                 
    }    
    
    //显示播放时间     
    OLEDShow0Num(0, 36, time / 60, 2, 12, 1);
    OLEDShowChar(12, 36, ':', 12, 1);
    OLEDShow0Num(18, 36, time % 60, 2, 12, 1);
    OLEDShowChar(30, 36, '/', 12, 1);
//    LCD_ShowxNum(30, 210, time / 60, 2, 16, 0X80);    //分钟
//    LCD_ShowChar(30 + 16, 210, ':', 16, 0);
//    LCD_ShowxNum(30 + 24, 210, time % 60, 2, 16, 0X80);  //秒钟    
//    LCD_ShowChar(30 + 40, 210, '/', 16, 0);       
    
    //显示总时间
    if(f_kbps)
    {
      time = (lenth / f_kbps) / 125;//得到秒钟数   (文件长度(字节)/(1000/8)/比特率=持续秒钟数        
    }
    else 
    {
      time = 0;//非法位率    
    }
    
    OLEDShow0Num(36, 36, time / 60, 2, 12, 1);
    OLEDShowChar(48, 36, ':', 12, 1);
    OLEDShow0Num(54, 36, time % 60, 2, 12, 1);
    
    OLEDShowNum(72, 36, f_kbps, 3, 12, 1);
    OLEDShowString(90, 36, "Kbps", 12, 1);
    OLEDRefreshGRAM();
//    LCD_ShowxNum(30 + 48, 210, time / 60, 2, 16, 0X80);  //分钟
//    LCD_ShowChar(30 + 64, 210, ':', 16, 0);
//    LCD_ShowxNum(30 + 72, 210, time % 60, 2, 16, 0X80);  //秒钟            
//    //显示位率         
//    LCD_ShowxNum(30 + 110, 210, f_kbps, 3, 16, 0X80);   //显示位率   
//    LCD_ShowString(30 + 134, 210, 200, 16, 16, "Kbps");        
//    LED0 = !LED0;    //DS0翻转
  }        
}      

/*********************************************************************************************************
* 函数名称: mp3_get_tnum
* 函数功能: 得到path路径下,目标文件的总个数
* 输入参数: path:路径  
* 输出参数: void
* 返 回 值: 总有效文件数
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u16 mp3_get_tnum(u8 *path)
{    
  u8 res;
  u16 rval = 0;
  DIR tdir;       //临时目录
  FILINFO tfileinfo;  //临时文件信息    
  u8 *fn;     
  
  res = f_opendir(&tdir, (const TCHAR*)path); //打开目录
  tfileinfo.lfsize = _MAX_LFN * 2 + 1;            //长文件名最大长度
  tfileinfo.lfname = MallocMemExt(USER_DEFINE_SRAMIN, tfileinfo.lfsize);  //为长文件缓存区分配内存
  
  if(res == FR_OK&&tfileinfo.lfname != NULL)
  {
    while(1)//查询总的有效文件数
    {
      res = f_readdir(&tdir, &tfileinfo);           //读取目录下的一个文件
      if(res != FR_OK || tfileinfo.fname[0] == 0)break;  //错误了/到末尾了,退出      
       fn = (u8*)(*tfileinfo.lfname ? tfileinfo.lfname : tfileinfo.fname);       
      res = f_typetell(fn);  
      if((res & 0XF0) == 0X40)//取高四位,看看是不是音乐文件  
      {
        rval++;//有效文件数增加1
      }      
    }  
  } 
  
  FreeMemExt(USER_DEFINE_SRAMIN, tfileinfo.lfname);
  
  return rval;
}

/*********************************************************************************************************
* 函数名称: mp3_play
* 函数功能: 播放音乐
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void mp3_play(void)
{
  u8 res;
  DIR mp3dir;         //目录
  FILINFO mp3fileinfo;//文件信息
  u8 *fn;             //长文件名
  u8 *pname;          //带路径的文件名
  u16 totmp3num;      //音乐文件总数
  u16 curindex;       //图片当前索引
  u8 key;             //键值      
  u16 temp;
  u16 *mp3indextbl;   //音乐索引表 
  
  while(f_opendir(&mp3dir, "0:/MUSIC"))//打开音乐文件夹
  {      
    Show_Str(0, 12, 128, 24, "MUSIC文件夹错误!", 12, 1);
    OLEDRefreshGRAM();
    DelayNms(200);                       
  }        
  
  totmp3num = mp3_get_tnum("0:/MUSIC"); //得到总有效文件数
  
  while(totmp3num == NULL)//音乐文件总数为0    
  {      
    Show_Str(0, 12, 128, 24, "没有音乐文件!", 12, 1);
    OLEDRefreshGRAM();
    DelayNms(200);                         
  }    
   
  mp3fileinfo.lfsize = _MAX_LFN * 2 + 1;            //长文件名最大长度
  mp3fileinfo.lfname = MallocMemExt(USER_DEFINE_SRAMIN, mp3fileinfo.lfsize);  //为长文件缓存区分配内存
  pname = MallocMemExt(USER_DEFINE_SRAMIN, mp3fileinfo.lfsize);        //为带路径的文件名分配内存
  mp3indextbl = MallocMemExt(USER_DEFINE_SRAMIN, 2 * totmp3num);        //申请2*totmp3num个字节的内存,用于存放音乐文件索引
  
  while(mp3fileinfo.lfname == NULL || pname == NULL || mp3indextbl == NULL)//内存分配出错
  {      
    Show_Str(0, 12, 128, 24, "内存分配失败!", 12, 1);
    OLEDRefreshGRAM();
    DelayNms(200);          
  } 
  
  VS_HD_Reset();
  VS_Soft_Reset();
  vsset.mvol = 220;            //默认设置音量为220.
  mp3_vol_show((vsset.mvol - 100) / 5);  //音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30   
  
  //记录索引
  res = f_opendir(&mp3dir, "0:/MUSIC"); //打开目录
  
  if(res == FR_OK)
  {
    curindex = 0;//当前索引为0
    
    while(1)//全部查询一遍
    {
      temp = mp3dir.index;                //记录当前index
      res = f_readdir(&mp3dir, &mp3fileinfo);           //读取目录下的一个文件
      if(res != FR_OK || mp3fileinfo.fname[0] == 0)
      {
        break;  //错误了/到末尾了,退出      
      }
      fn = (u8*)(*mp3fileinfo.lfname ? mp3fileinfo.lfname : mp3fileinfo.fname);       
      res = f_typetell(fn);  
      
      if((res & 0XF0) == 0X40)//取高四位,看看是不是音乐文件  
      {
        mp3indextbl[curindex] = temp;//记录索引
        curindex++;
      }      
    } 
  }  
  
  curindex = 0;                      //从0开始显示
  res = f_opendir(&mp3dir, (const TCHAR*)"0:/MUSIC");   //打开目录
  
  while(res == FR_OK)//打开成功
  {  
    dir_sdi(&mp3dir, mp3indextbl[curindex]);      //改变当前目录索引     
    res = f_readdir(&mp3dir,&mp3fileinfo);           //读取目录下的一个文件
    
    if(res != FR_OK || mp3fileinfo.fname[0] == 0)
    {
      break;  //错误了/到末尾了,退出
    }
    
    fn = (u8*)(*mp3fileinfo.lfname ? mp3fileinfo.lfname : mp3fileinfo.fname);      
    
    strcpy((char*)pname, "0:/MUSIC/");        //复制路径(目录)
    strcat((char*)pname, (const char*)fn);        //将文件名接在后面
    
//    OLEDClear();        //清除之前的显示
    
    OLEDClearArea(0, 12, 128, 36);
    Show_Str(0, 12, 128, 24, fn, 12, 1);        //显示歌曲名字 
    OLEDRefreshGRAM();
    
    mp3_index_show(curindex + 1, totmp3num);
    key = mp3_play_song(pname);              //播放这个MP3  
    
    if(key == 2)    //上一曲
    {
      if(curindex)
      {
        curindex--;
      }
      else 
      {
        curindex = totmp3num - 1;
      }
    }
    else if(key <= 1)//下一曲
    {
      curindex++;     
      
      if(curindex >= totmp3num)
      {
        curindex = 0;//到末尾的时候,自动从头开始
      }
    }
    else if(key == 3)
    {      
      NowInterFace = 0;
      NowPointAt   = 5;
      UpdateMask(NowPointAt);
      ShowMenu();
      
      break;
    }
    else 
    {
      break;  //产生了错误    
    }
  }   
  
  FreeMemExt(USER_DEFINE_SRAMIN, mp3fileinfo.lfname);   //释放内存          
  FreeMemExt(USER_DEFINE_SRAMIN, pname);                //释放内存          
  FreeMemExt(USER_DEFINE_SRAMIN, mp3indextbl);          //释放内存   
}

/*********************************************************************************************************
* 函数名称: mp3_play_song
* 函数功能: 
* 输入参数: pname，歌曲名
* 输出参数: void
* 返 回 值: 0,正常播放完成；1,下一曲；2,上一曲；0XFF,出现错误了
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
u8 mp3_play_song(u8 *pname)
{   
  FIL* fmp3;
  u16  br;
  u8   res,rval;    
  u8   *databuf;            
  u16  i = 0;         
         
  musicKey = 0;
  rval = 0;      
  fmp3 = (FIL*)MallocMemExt(USER_DEFINE_SRAMIN, sizeof(FIL));//申请内存
  databuf = (u8*)MallocMemExt(USER_DEFINE_SRAMIN, 4096);    //开辟4096字节的内存区域
  
  if(databuf == NULL || fmp3 == NULL)
  {
    rval = 0XFF ;//内存申请失败.
  }
  
  if(rval == 0)
  {    
    VS_Restart_Play();            //重启播放 
    VS_Set_All();                  //设置音量等信息        
    VS_Reset_DecodeTime();          //复位解码时间     
    res = f_typetell(pname);            //得到文件后缀     
    
    if(res == 0x4c)//如果是flac,加载patch
    {  
      VS_Load_Patch((u16*)vs1053b_patch, VS1053B_PATCHLEN);
    }     
    
    res = f_open(fmp3, (const TCHAR*)pname, FA_READ);//打开文件 
    
    if(res == 0)//打开成功.
    { 
      VS_SPI_SpeedHigh();  //高速   
      
      while(rval == 0)
      {
        res = f_read(fmp3, databuf, 4096, (UINT*)&br);//读出4096个字节  
        i = 0;
        
        do//主播放循环
        {    
          if(VS_Send_MusicData(databuf + i) == 0)//给VS10XX发送音频数据
          {
            i += 32;
          }
          else   
          {
            ScanKeyOne(KEY_NAME_KEY0, ProcKeyUpKey0, ProcKeyDownKey0);      //扫描得到按键0的状态
            ScanKeyOne(KEY_NAME_KEY1, ProcKeyUpKey1, ProcKeyDownKey1);      //扫描得到按键1的状态
            ScanKeyOne(KEY_NAME_KEY2, ProcKeyUpKey2, ProcKeyDownKey2);      //扫描得到按键2的状态         
                      
            switch(musicKey)
            {
              case NEXT_SONG:
                rval = 1;    //下一曲
              
                break;
              case PREV_SONG:
                rval = 2;    //上一曲
              
                break;
              case VOL_UP:  //音量增加
                if(vsset.mvol < 250)
                {
                  vsset.mvol += 5;
                  VS_Set_Vol(vsset.mvol);  
                }
                else 
                {
                  vsset.mvol = 250;
                }
                mp3_vol_show((vsset.mvol - 100) / 5);  //音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30   
                musicKey = 0;
                
                break;
              case VOL_DOWN:  //音量减
                if(vsset.mvol>100)
                {
                  vsset.mvol -= 5;
                  VS_Set_Vol(vsset.mvol);  
                }
                else 
                {
                  vsset.mvol = 100;
                }
                mp3_vol_show((vsset.mvol - 100) / 5);  //音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30  
                musicKey = 0;
                
                break;
              case PAUSE:
                while(musicKey == PAUSE)
                {
                  ScanKeyOne(KEY_NAME_KEY1, ProcKeyUpKey1, ProcKeyDownKey1);      //扫描得到按键1的状态
                }
              
                break;
              case QUIT:
                rval = 3;
              
                if(AlarmStop == 1)
                {
                  AlarmStop = 0;
                }
                
                break;
            }
            
            if(Playmusic == 1)
            {
              
            }
            else
            {
              mp3_msg_show(fmp3->fsize);//显示信息      
            }           
          }            
        }while(i < 4096);//循环发送4096个字节 
          
        if(br != 4096 || res != 0)
        {
          rval = 0;
          
          break;//读完了.      
        }
      }
      f_close(fmp3);
    }
    else 
    {
      rval = 0XFF;//出现错误         
    }
  }      
  
  FreeMemExt(USER_DEFINE_SRAMIN, databuf);                   
  FreeMemExt(USER_DEFINE_SRAMIN, fmp3);
  
  return rval;                   
}

/*********************************************************************************************************
* 函数名称: PlaySystemMusic
* 函数功能: 播放系统音乐（开机或者闹铃）
* 输入参数: pname，歌曲名
* 输出参数: void
* 返 回 值: 0,正常播放完成；1,下一曲；2,上一曲；0XFF,出现错误了
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/
void PlaySystemMusic(u8 *pname)
{
  VS_HD_Reset();
  VS_Soft_Reset();
  vsset.mvol = 220;            //默认设置音量为220.
  
  Playmusic = 1;
  mp3_play_song(pname);
  Playmusic = 0;
}
