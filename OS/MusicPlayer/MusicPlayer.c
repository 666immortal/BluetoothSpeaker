/*********************************************************************************************************
* ģ������: MusicPlayer.c
* ժ    Ҫ: 
* ��ǰ�汾: 1.0.0
* ��    ��: 
* �������: 
* ��    ��:
* ע    ��: none                                                                  
**********************************************************************************************************
* ȡ���汾: 
* ��    ��:
* �������: 
* �޸�����:
* �޸��ļ�: 
*********************************************************************************************************/

/*********************************************************************************************************
*                                              ����ͷ�ļ�
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
*                                              �궨��
*********************************************************************************************************/

/*********************************************************************************************************
*                                              ö�ٽṹ�嶨��
*********************************************************************************************************/

/*********************************************************************************************************
*                                              �ڲ�����
*********************************************************************************************************/
u16 f_kbps = 0;//�����ļ�λ��
u8  musicKey = 0;
u8  Playmusic = 0;

extern u8 NowInterFace;
extern u8 NowPointAt;
extern u8 AlarmStop;

/*********************************************************************************************************
*                                              �ڲ���������
*********************************************************************************************************/

/*********************************************************************************************************
*                                              �ڲ�����ʵ��
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API����ʵ��
*********************************************************************************************************/
/*********************************************************************************************************
* ��������: mp3_index_show
* ��������: ��ʾ��Ŀ����
* �������: index:��ǰ������total:���ļ���
* �������: void
* �� �� ֵ: void
* ��������: 20XX��XX��XX��
* ע    ��: 
*********************************************************************************************************/
void mp3_index_show(u16 index,u16 total)
{
  //��ʾ��ǰ��Ŀ������,������Ŀ��
  OLEDShow0Num(0, 48, index, 3, 12, 1);
  OLEDShowChar(18, 48, '/', 12, 1);
  OLEDShow0Num(24, 48, total, 3, 12, 1);
  OLEDRefreshGRAM();
//  LCD_ShowxNum(30 + 0, 230, index, 3, 16, 0X80);    //����
//  LCD_ShowChar(30 + 24, 230, '/', 16, 0);
//  LCD_ShowxNum(30 + 32, 230, total, 3, 16, 0X80);   //����Ŀ              
}

/*********************************************************************************************************
* ��������: mp3_vol_show
* ��������: ��ʾ��ǰ����
* �������: vol,����
* �������: void
* �� �� ֵ: void
* ��������: 2018��03��01��
* ע    ��: 
*********************************************************************************************************/
void mp3_vol_show(u8 vol)
{  
  OLEDShowString(54, 48, "VOL:", 12, 1);
  OLEDShowNum(84, 48, vol, 2, 12, 1);
  OLEDRefreshGRAM();
  
//  LCD_ShowString(30 + 110, 230, 200, 16, 16, "VOL:");        
//  LCD_ShowxNum(30 + 142, 230, vol, 2, 16, 0X80);   //��ʾ����   
}

/*********************************************************************************************************
* ��������: mp3_msg_show
* ��������: ��ʾ����ʱ��,������ ��Ϣ
* �������: lenth:�����ܳ���
* �������: void
* �� �� ֵ: void
* ��������: 2018��03��01��
* ע    ��: 
*********************************************************************************************************/  
void mp3_msg_show(u32 lenth)
{  
  static u16 playtime = 0;//����ʱ����       
  u16 time = 0;// ʱ�����
  u16 temp = 0;  
  
  if(f_kbps == 0xffff)//δ���¹�
  {
    playtime = 0;
    f_kbps = VS_Get_HeadInfo();     //��ñ�����
  }     
  
  time = VS_Get_DecodeTime(); //�õ�����ʱ��
  
  if(playtime == 0)
  {
    playtime = time;
  }
  else if((time != playtime) && (time != 0))//1sʱ�䵽,������ʾ����
  {
    playtime = time;//����ʱ��                
    temp = VS_Get_HeadInfo(); //��ñ�����              
    if(temp != f_kbps)
    {
      f_kbps = temp;//����KBPS                 
    }    
    
    //��ʾ����ʱ��     
    OLEDShow0Num(0, 36, time / 60, 2, 12, 1);
    OLEDShowChar(12, 36, ':', 12, 1);
    OLEDShow0Num(18, 36, time % 60, 2, 12, 1);
    OLEDShowChar(30, 36, '/', 12, 1);
//    LCD_ShowxNum(30, 210, time / 60, 2, 16, 0X80);    //����
//    LCD_ShowChar(30 + 16, 210, ':', 16, 0);
//    LCD_ShowxNum(30 + 24, 210, time % 60, 2, 16, 0X80);  //����    
//    LCD_ShowChar(30 + 40, 210, '/', 16, 0);       
    
    //��ʾ��ʱ��
    if(f_kbps)
    {
      time = (lenth / f_kbps) / 125;//�õ�������   (�ļ�����(�ֽ�)/(1000/8)/������=����������        
    }
    else 
    {
      time = 0;//�Ƿ�λ��    
    }
    
    OLEDShow0Num(36, 36, time / 60, 2, 12, 1);
    OLEDShowChar(48, 36, ':', 12, 1);
    OLEDShow0Num(54, 36, time % 60, 2, 12, 1);
    
    OLEDShowNum(72, 36, f_kbps, 3, 12, 1);
    OLEDShowString(90, 36, "Kbps", 12, 1);
    OLEDRefreshGRAM();
//    LCD_ShowxNum(30 + 48, 210, time / 60, 2, 16, 0X80);  //����
//    LCD_ShowChar(30 + 64, 210, ':', 16, 0);
//    LCD_ShowxNum(30 + 72, 210, time % 60, 2, 16, 0X80);  //����            
//    //��ʾλ��         
//    LCD_ShowxNum(30 + 110, 210, f_kbps, 3, 16, 0X80);   //��ʾλ��   
//    LCD_ShowString(30 + 134, 210, 200, 16, 16, "Kbps");        
//    LED0 = !LED0;    //DS0��ת
  }        
}      

/*********************************************************************************************************
* ��������: mp3_get_tnum
* ��������: �õ�path·����,Ŀ���ļ����ܸ���
* �������: path:·��  
* �������: void
* �� �� ֵ: ����Ч�ļ���
* ��������: 2018��03��01��
* ע    ��: 
*********************************************************************************************************/
u16 mp3_get_tnum(u8 *path)
{    
  u8 res;
  u16 rval = 0;
  DIR tdir;       //��ʱĿ¼
  FILINFO tfileinfo;  //��ʱ�ļ���Ϣ    
  u8 *fn;     
  
  res = f_opendir(&tdir, (const TCHAR*)path); //��Ŀ¼
  tfileinfo.lfsize = _MAX_LFN * 2 + 1;            //���ļ�����󳤶�
  tfileinfo.lfname = MallocMemExt(USER_DEFINE_SRAMIN, tfileinfo.lfsize);  //Ϊ���ļ������������ڴ�
  
  if(res == FR_OK&&tfileinfo.lfname != NULL)
  {
    while(1)//��ѯ�ܵ���Ч�ļ���
    {
      res = f_readdir(&tdir, &tfileinfo);           //��ȡĿ¼�µ�һ���ļ�
      if(res != FR_OK || tfileinfo.fname[0] == 0)break;  //������/��ĩβ��,�˳�      
       fn = (u8*)(*tfileinfo.lfname ? tfileinfo.lfname : tfileinfo.fname);       
      res = f_typetell(fn);  
      if((res & 0XF0) == 0X40)//ȡ����λ,�����ǲ��������ļ�  
      {
        rval++;//��Ч�ļ�������1
      }      
    }  
  } 
  
  FreeMemExt(USER_DEFINE_SRAMIN, tfileinfo.lfname);
  
  return rval;
}

/*********************************************************************************************************
* ��������: mp3_play
* ��������: ��������
* �������: void
* �������: void
* �� �� ֵ: void
* ��������: 2018��03��01��
* ע    ��: 
*********************************************************************************************************/
void mp3_play(void)
{
  u8 res;
  DIR mp3dir;         //Ŀ¼
  FILINFO mp3fileinfo;//�ļ���Ϣ
  u8 *fn;             //���ļ���
  u8 *pname;          //��·�����ļ���
  u16 totmp3num;      //�����ļ�����
  u16 curindex;       //ͼƬ��ǰ����
  u8 key;             //��ֵ      
  u16 temp;
  u16 *mp3indextbl;   //���������� 
  
  while(f_opendir(&mp3dir, "0:/MUSIC"))//�������ļ���
  {      
    Show_Str(0, 12, 128, 24, "MUSIC�ļ��д���!", 12, 1);
    OLEDRefreshGRAM();
    DelayNms(200);                       
  }        
  
  totmp3num = mp3_get_tnum("0:/MUSIC"); //�õ�����Ч�ļ���
  
  while(totmp3num == NULL)//�����ļ�����Ϊ0    
  {      
    Show_Str(0, 12, 128, 24, "û�������ļ�!", 12, 1);
    OLEDRefreshGRAM();
    DelayNms(200);                         
  }    
   
  mp3fileinfo.lfsize = _MAX_LFN * 2 + 1;            //���ļ�����󳤶�
  mp3fileinfo.lfname = MallocMemExt(USER_DEFINE_SRAMIN, mp3fileinfo.lfsize);  //Ϊ���ļ������������ڴ�
  pname = MallocMemExt(USER_DEFINE_SRAMIN, mp3fileinfo.lfsize);        //Ϊ��·�����ļ��������ڴ�
  mp3indextbl = MallocMemExt(USER_DEFINE_SRAMIN, 2 * totmp3num);        //����2*totmp3num���ֽڵ��ڴ�,���ڴ�������ļ�����
  
  while(mp3fileinfo.lfname == NULL || pname == NULL || mp3indextbl == NULL)//�ڴ�������
  {      
    Show_Str(0, 12, 128, 24, "�ڴ����ʧ��!", 12, 1);
    OLEDRefreshGRAM();
    DelayNms(200);          
  } 
  
  VS_HD_Reset();
  VS_Soft_Reset();
  vsset.mvol = 220;            //Ĭ����������Ϊ220.
  mp3_vol_show((vsset.mvol - 100) / 5);  //����������:100~250,��ʾ��ʱ��,���չ�ʽ(vol-100)/5,��ʾ,Ҳ����0~30   
  
  //��¼����
  res = f_opendir(&mp3dir, "0:/MUSIC"); //��Ŀ¼
  
  if(res == FR_OK)
  {
    curindex = 0;//��ǰ����Ϊ0
    
    while(1)//ȫ����ѯһ��
    {
      temp = mp3dir.index;                //��¼��ǰindex
      res = f_readdir(&mp3dir, &mp3fileinfo);           //��ȡĿ¼�µ�һ���ļ�
      if(res != FR_OK || mp3fileinfo.fname[0] == 0)
      {
        break;  //������/��ĩβ��,�˳�      
      }
      fn = (u8*)(*mp3fileinfo.lfname ? mp3fileinfo.lfname : mp3fileinfo.fname);       
      res = f_typetell(fn);  
      
      if((res & 0XF0) == 0X40)//ȡ����λ,�����ǲ��������ļ�  
      {
        mp3indextbl[curindex] = temp;//��¼����
        curindex++;
      }      
    } 
  }  
  
  curindex = 0;                      //��0��ʼ��ʾ
  res = f_opendir(&mp3dir, (const TCHAR*)"0:/MUSIC");   //��Ŀ¼
  
  while(res == FR_OK)//�򿪳ɹ�
  {  
    dir_sdi(&mp3dir, mp3indextbl[curindex]);      //�ı䵱ǰĿ¼����     
    res = f_readdir(&mp3dir,&mp3fileinfo);           //��ȡĿ¼�µ�һ���ļ�
    
    if(res != FR_OK || mp3fileinfo.fname[0] == 0)
    {
      break;  //������/��ĩβ��,�˳�
    }
    
    fn = (u8*)(*mp3fileinfo.lfname ? mp3fileinfo.lfname : mp3fileinfo.fname);      
    
    strcpy((char*)pname, "0:/MUSIC/");        //����·��(Ŀ¼)
    strcat((char*)pname, (const char*)fn);        //���ļ������ں���
    
//    OLEDClear();        //���֮ǰ����ʾ
    
    OLEDClearArea(0, 12, 128, 36);
    Show_Str(0, 12, 128, 24, fn, 12, 1);        //��ʾ�������� 
    OLEDRefreshGRAM();
    
    mp3_index_show(curindex + 1, totmp3num);
    key = mp3_play_song(pname);              //�������MP3  
    
    if(key == 2)    //��һ��
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
    else if(key <= 1)//��һ��
    {
      curindex++;     
      
      if(curindex >= totmp3num)
      {
        curindex = 0;//��ĩβ��ʱ��,�Զ���ͷ��ʼ
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
      break;  //�����˴���    
    }
  }   
  
  FreeMemExt(USER_DEFINE_SRAMIN, mp3fileinfo.lfname);   //�ͷ��ڴ�          
  FreeMemExt(USER_DEFINE_SRAMIN, pname);                //�ͷ��ڴ�          
  FreeMemExt(USER_DEFINE_SRAMIN, mp3indextbl);          //�ͷ��ڴ�   
}

/*********************************************************************************************************
* ��������: mp3_play_song
* ��������: 
* �������: pname��������
* �������: void
* �� �� ֵ: 0,����������ɣ�1,��һ����2,��һ����0XFF,���ִ�����
* ��������: 2018��03��01��
* ע    ��: 
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
  fmp3 = (FIL*)MallocMemExt(USER_DEFINE_SRAMIN, sizeof(FIL));//�����ڴ�
  databuf = (u8*)MallocMemExt(USER_DEFINE_SRAMIN, 4096);    //����4096�ֽڵ��ڴ�����
  
  if(databuf == NULL || fmp3 == NULL)
  {
    rval = 0XFF ;//�ڴ�����ʧ��.
  }
  
  if(rval == 0)
  {    
    VS_Restart_Play();            //�������� 
    VS_Set_All();                  //������������Ϣ        
    VS_Reset_DecodeTime();          //��λ����ʱ��     
    res = f_typetell(pname);            //�õ��ļ���׺     
    
    if(res == 0x4c)//�����flac,����patch
    {  
      VS_Load_Patch((u16*)vs1053b_patch, VS1053B_PATCHLEN);
    }     
    
    res = f_open(fmp3, (const TCHAR*)pname, FA_READ);//���ļ� 
    
    if(res == 0)//�򿪳ɹ�.
    { 
      VS_SPI_SpeedHigh();  //����   
      
      while(rval == 0)
      {
        res = f_read(fmp3, databuf, 4096, (UINT*)&br);//����4096���ֽ�  
        i = 0;
        
        do//������ѭ��
        {    
          if(VS_Send_MusicData(databuf + i) == 0)//��VS10XX������Ƶ����
          {
            i += 32;
          }
          else   
          {
            ScanKeyOne(KEY_NAME_KEY0, ProcKeyUpKey0, ProcKeyDownKey0);      //ɨ��õ�����0��״̬
            ScanKeyOne(KEY_NAME_KEY1, ProcKeyUpKey1, ProcKeyDownKey1);      //ɨ��õ�����1��״̬
            ScanKeyOne(KEY_NAME_KEY2, ProcKeyUpKey2, ProcKeyDownKey2);      //ɨ��õ�����2��״̬         
                      
            switch(musicKey)
            {
              case NEXT_SONG:
                rval = 1;    //��һ��
              
                break;
              case PREV_SONG:
                rval = 2;    //��һ��
              
                break;
              case VOL_UP:  //��������
                if(vsset.mvol < 250)
                {
                  vsset.mvol += 5;
                  VS_Set_Vol(vsset.mvol);  
                }
                else 
                {
                  vsset.mvol = 250;
                }
                mp3_vol_show((vsset.mvol - 100) / 5);  //����������:100~250,��ʾ��ʱ��,���չ�ʽ(vol-100)/5,��ʾ,Ҳ����0~30   
                musicKey = 0;
                
                break;
              case VOL_DOWN:  //������
                if(vsset.mvol>100)
                {
                  vsset.mvol -= 5;
                  VS_Set_Vol(vsset.mvol);  
                }
                else 
                {
                  vsset.mvol = 100;
                }
                mp3_vol_show((vsset.mvol - 100) / 5);  //����������:100~250,��ʾ��ʱ��,���չ�ʽ(vol-100)/5,��ʾ,Ҳ����0~30  
                musicKey = 0;
                
                break;
              case PAUSE:
                while(musicKey == PAUSE)
                {
                  ScanKeyOne(KEY_NAME_KEY1, ProcKeyUpKey1, ProcKeyDownKey1);      //ɨ��õ�����1��״̬
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
              mp3_msg_show(fmp3->fsize);//��ʾ��Ϣ      
            }           
          }            
        }while(i < 4096);//ѭ������4096���ֽ� 
          
        if(br != 4096 || res != 0)
        {
          rval = 0;
          
          break;//������.      
        }
      }
      f_close(fmp3);
    }
    else 
    {
      rval = 0XFF;//���ִ���         
    }
  }      
  
  FreeMemExt(USER_DEFINE_SRAMIN, databuf);                   
  FreeMemExt(USER_DEFINE_SRAMIN, fmp3);
  
  return rval;                   
}

/*********************************************************************************************************
* ��������: PlaySystemMusic
* ��������: ����ϵͳ���֣������������壩
* �������: pname��������
* �������: void
* �� �� ֵ: 0,����������ɣ�1,��һ����2,��һ����0XFF,���ִ�����
* ��������: 2018��03��01��
* ע    ��: 
*********************************************************************************************************/
void PlaySystemMusic(u8 *pname)
{
  VS_HD_Reset();
  VS_Soft_Reset();
  vsset.mvol = 220;            //Ĭ����������Ϊ220.
  
  Playmusic = 1;
  mp3_play_song(pname);
  Playmusic = 0;
}
