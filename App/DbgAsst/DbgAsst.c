/*********************************************************************************************************
* 模块名称: DbgAsst.c
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
#include "DbgAsst.h"
#include "UART.h"
#include "CheckLineFeed.h"
#include "LED.h"
#include "ProcString.h"
#include "Timer.h"
#include "DbgFat.h"
#include "RTC.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/
//函数信息列表结构体
typedef struct
{
  void* pFunc;          //函数指针
  const u8* pFuncName;  //函数名(查找串)  
}StructNameTab;

//系统命令，数组里存放的是字符串的指针地址，所有的指令都是大小写敏感的
static u8* s_arrSysCmdTbl[]=
{
  "?",                  //打印使用帮助信息
  "help",               //同“？”一样，打印使用帮助信息
  "list",               //打印所有可调用的函数
  "id",                 //用于获取各个函数的入口地址
  "hex",                //当带参数的时候，hex用于设置串口显示数据格式为16进制；带参数时，用于执行进制转换
  "dec",                //当带参数的时候，dec用于设置串口显示数据格式为10进制；带参数时，用于执行进制转换
  "runtime",            //用于函数执行时间统计功能的开启和关闭
};  

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
u8  g_arrPara[PARM_LEN];      //函数的参数的起始位置，这是一个全局变量

static  u8  s_iFuncNum;       //函数数量
static  u8  s_iParaNum;       //参数数量
static  u8  s_iFuncID;        //函数ID
static  u8  s_iParaDispType;  //参数显示类型(非字符串参数):0,10进制;1,16进制;
static  u8  s_iRunTimeFlag;   //是否统计函数执行时间：0,不统计函数执行时间;1,统计函数执行时间
static  u16 s_iParaType;      //参数的类型：1-字符串，0-数字;按照参数顺序逐位记录

//函数名列表数组初始化，用户自行添加，
static StructNameTab s_structNameTab[] =
{
  (void*)ReadAddr,"u32 ReadAddr(u32 addr)",
  (void*)WriteAddr,"void WriteAddr(u32 addr, u32 val)",
    
  (void*)mf_mount,"u8 mf_mount(u8 drv)", 
	(void*)mf_open,"u8 mf_open(u8*path,u8 mode)", 
	(void*)mf_close,"u8 mf_close(void)", 
	(void*)mf_read,"u8 mf_read(u16 len)", 
	(void*)mf_write,"u8 mf_write(u8*dat,u16 len)", 
	(void*)mf_opendir,"u8 mf_opendir(u8* path)", 
	(void*)mf_readdir,"u8 mf_readdir(void)", 
	(void*)mf_scan_files,"u8 mf_scan_files(u8 * path)", 
	(void*)mf_showfree,"u32 mf_showfree(u8 *drv)", 
	(void*)mf_lseek,"u8 mf_lseek(u32 offset)", 
	(void*)mf_tell,"u32 mf_tell(void)", 
	(void*)mf_size,"u32 mf_size(void)", 
	(void*)mf_mkdir,"u8 mf_mkdir(u8*pname)", 
	(void*)mf_fmkfs,"u8 mf_fmkfs(u8 drv,u8 mode,u16 au)", 
	(void*)mf_unlink,"u8 mf_unlink(u8 *pname)", 
	(void*)mf_rename,"u8 mf_rename(u8 *oldname,u8* newname)", 
	(void*)mf_gets,"void mf_gets(u16 size)", 
	(void*)mf_putc,"u8 mf_putc(u8 c)", 
	(void*)mf_puts,"u8 mf_puts(u8*c)", 	
  (void*)SetRTC,"u8 SetRTC(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)",
  (void*)SetAlarm,"u8 SetAlarm(u8 hour, u8 min)",
};

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static u8   GetFuncInfo(u8* pStr);  //获取函数信息
static void ExeFunc(void);          //执行函数
static u8   ProcSysCmd(u8* pStr);   //处理系统命令

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: GetFuncInfo
* 函数功能: 从pStr中获取函数名,id,及参数信息
* 输入参数: pStr:字符串指针 
* 输出参数: void
* 返 回 值: 0,成功处理;其他,错误代码
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/	 
static u8 GetFuncInfo(u8* pStr) 
{
  u8 sts;                                 //用于存放函数运行状态的标志位
  u8 i;                                   //循环计数器
  u8 retValFlag;                          //是否需要显示返回值，这个变量只作为函数接收参数，只接收，不使用
  u8 recFuncParaNum;                      //从串口接收到的函数的参数数量  
  u8 recFuncName[MAX_FUNC_NAME_LEN];      //从串口接收到的函数的函数名 
  u8 registedFuncParaNum;                 //已经注册的函数的参数数量
  u8 registedFuncName[MAX_FUNC_NAME_LEN]; //已注册的函数的函数名
  
  //得到接收到的数据的函数名、参数个数和是否需要返回值标志
  sts = GetFuncName(pStr, recFuncName, &recFuncParaNum, &retValFlag);	  
  
  if(sts)                                 //如果GetFuncName运行失败
  {
    return sts;                           //本地函数解析有误
  }  
  
  for(i = 0; i < s_iFuncNum; i++)         //遍历已注册的所有函数
  {
    //得到本地函数名及参数个数
    sts = GetFuncName((u8*)s_structNameTab[i].pFuncName, registedFuncName, &registedFuncParaNum, &retValFlag);
  
    if(sts)                               //如果GetFuncName运行失败
    {
      return sts;                         //本地函数解析有误  
    }    
  
    if(StrCmp(registedFuncName, recFuncName) == 0)  //将已注册的函数名和接受到的字符串比较
    {
      if(registedFuncParaNum > recFuncParaNum)      //如果已注册的函数参数数量大于输入的参数数量
      {
        return DBG_ASST_PARA_ERR;         //参数错误(输入参数比已注册函数的参数少)  
      }  
 
      s_iFuncID = i;                      //记录函数ID
  
      break;                              //跳出循环
    } 
  }
  
  if(i == s_iFuncNum)                     //i等于s_iFuncNum，即对比完所有函数
  {
    return DBG_ASST_FUNC_MISMATCH;        //未找到匹配的函数  
  }  
  
  sts = GetAllFuncPara(pStr, &i);         //得到函数参数个数
  
  if(sts)                                 //不为0
  {
    return sts;                           //本地函数解析有误
  }  
  
  s_iParaNum = i;                         //参数个数记录
  
  return DBG_ASST_OK;                     //识别成功
}

/*********************************************************************************************************
* 函数名称: ExeFunc
* 函数功能: 执行函数，该函数用于最终执行从串口收到的有效函数
* 输入参数: pStr 
* 输出参数: void
* 返 回 值: 0,成功处理;其他,错误代码
* 创建日期: 2018年03月01日
* 注    意: 最多支持10个参数的函数,更多的参数支持也很容易实现.不过用的很少.一般5个左右的参数的函数已经很少见了.
*           该函数会在串口打印执行情况.以:"函数名(参数1，参数2...参数N)=返回值".的形式打印.
*           当所执行的函数没有返回值的时候,所打印的返回值是一个无意义的数据.
*********************************************************************************************************/ 
static void ExeFunc(void)
{
  u8  id;                         //用于存放函数的存放数组中的位置
  u8  i;                          //循环计数器
  u32 res;                        //用于存放返回值
  u32 temp[MAX_PARA_NUM];         //参数转换,使之支持了字符串 
  u8  sfname[MAX_FUNC_NAME_LEN];  //存放本地函数名
  u8  pnum;                       //存放参数的个数
  u8  rval;                       //是否需要返回值标志
  i32 funcExeTime = 0;            //函数执行时间
  
  id = s_iFuncID;                 //记录函数的存放数组中的位置
  
  if(id >= s_iFuncNum)            //如果超过记录的范围
  { 
    return;                       //跳出函数
  }
  
  //得到本地函数名,及参数个数 
  GetFuncName((u8*)s_structNameTab[id].pFuncName, sfname, &pnum, &rval);
  printf("\r\n%s(", sfname);    //打印正要执行的函数名和左括号
  
  for(i = 0; i < pnum; i++)     //循环打印所有的参数
  {
    if(s_iParaType & (1 << i))  //如果参数是字符串
    {
      printf("%c", '"');        //打印左双引号
      printf("%s", g_arrPara + GetParaPos(i));  //打印字符串
      printf("%c",'"');         //打印右双引号
      temp[i] = (u32)&(g_arrPara[GetParaPos(i)]);
    }
    else                        //如果参数是数字
    {
      //将参数存入temp的第i个位置
      temp[i] = *(u32*)(g_arrPara + GetParaPos(i));
      if(s_iParaDispType == SP_TYPE_DEC)
      {
        printf("%lu", temp[i]); //10进制参数显示  
      }  
      else 
      {
        printf("0X%X", temp[i]);//16进制参数显示
      }        
    }
    if(i != pnum - 1)
    {
      printf(",");              //打印完所有参数后，打印逗号
    }
  }
  
  printf(")");                  //打印完所有参数后，打印右括号
  ResetDbgAsstTimer();          //计时器清零,开始计时
  
  switch(s_iParaNum)
  {
    case 0://无参数(void类型)
      res = (*(u32(*)())s_structNameTab[id].pFunc)();
      break;
    case 1://有1个参数
      res = (*(u32(*)())s_structNameTab[id].pFunc)(temp[0]);
      break;
    case 2://有2个参数
      res = (*(u32(*)())s_structNameTab[id].pFunc)(temp[0], temp[1]);
      break;
    case 3://有3个参数
      res = (*(u32(*)())s_structNameTab[id].pFunc)(temp[0], temp[1], temp[2]);
      break;
    case 4://有4个参数
      res = (*(u32(*)())s_structNameTab[id].pFunc)(temp[0], temp[1], temp[2], temp[3]);
      break;
    case 5://有5个参数
      res = (*(u32(*)())s_structNameTab[id].pFunc)(temp[0], temp[1], temp[2], temp[3], temp[4]);
      break;
    case 6://有6个参数
      res = (*(u32(*)())s_structNameTab[id].pFunc)(temp[0], temp[1], temp[2], temp[3], temp[4],\
      temp[5]);
      break;
    case 7://有7个参数
      res = (*(u32(*)())s_structNameTab[id].pFunc)(temp[0], temp[1], temp[2], temp[3], temp[4],\
      temp[5], temp[6]);
      break;
    case 8://有8个参数
      res = (*(u32(*)())s_structNameTab[id].pFunc)(temp[0], temp[1], temp[2], temp[3], temp[4],\
      temp[5], temp[6], temp[7]);
      break;
    case 9://有9个参数
      res = (*(u32(*)())s_structNameTab[id].pFunc)(temp[0], temp[1], temp[2], temp[3], temp[4],\
      temp[5], temp[6], temp[7], temp[8]);
      break;
    case 10://有10个参数
      res = (*(u32(*)())s_structNameTab[id].pFunc)(temp[0], temp[1], temp[2], temp[3], temp[4],\
      temp[5], temp[6], temp[7], temp[8], temp[9]);
      break;
  }
  
  funcExeTime = GetDbgAsstTimer();    //获取函数执行时间
  
  if(rval == 1)                       //需要返回值.
  {
    if(s_iParaDispType==SP_TYPE_DEC)
    {
      printf("=%lu;\r\n", res);       //输出执行结果(10进制参数显示)
    }  
    else 
    {
      printf("=0X%X;\r\n", res);      //输出执行结果(16进制参数显示)  
    }   
  }
  else 
  {
    printf(";\r\n");                  //不需要返回值,打印分号，输出结束  
  }  
  
  if(s_iRunTimeFlag)                  //需要显示函数执行时间
  {      
    printf("Function Run Time:%dus\r\n", funcExeTime * 10);//打印函数执行时间     
  }
}

/*********************************************************************************************************
* 函数名称: ProcSysCmd
* 函数功能: 处理系统指令
* 输入参数: pStr 
* 输出参数: void
* 返 回 值: 0,成功处理;其他,错误代码
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/	 
static u8 ProcSysCmd(u8* pStr)
{
  u8 i;                             //循环计数器
  u8 arrFuncName[MAX_FUNC_NAME_LEN];//存放本地函数名
  u8 pnum;                          //存放参数的个数
  u8 rval;                          //是否需要返回值标志
  u32 res;                          //存放函数执行结果
  u16 paraType;                     //存放参数类型
  u8 sts;                           //用于存放函数运行状态的标志位                           
    
  res = GetCmdName(pStr, arrFuncName, &i, MAX_FUNC_NAME_LEN);//得到指令及指令长度
  
  if(res)
  {
    return DBG_ASST_FUNC_ERR;                       //错误的指令  
  }  
  
  pStr += i;                                        //指针移到指令后面一位

  for(i = 0; i < sizeof(s_arrSysCmdTbl) / 4; i++)   //支持的系统指令
  {
    if(StrCmp(arrFuncName, s_arrSysCmdTbl[i]) == 0) //与系统现有的指令做比较
    {
      break;
    }
  }
  
  switch(i)
  { 
    case 0:
    case 1:                                         //帮助指令
      printf("\r\n");                               //换行
      printf("------------------------USMART V3.1------------------------ \r\n");
      printf("    USMART是由ALIENTEK开发的一个灵巧的串口调试互交组件,通过 \r\n");
      printf("它,你可以通过串口助手调用程序里面的任何函数,并执行.因此,你可\r\n");
      printf("以随意更改函数的输入参数(支持数字(10/16进制)、字符串、函数入\r\n");	  
      printf("口地址等作为参数),单个函数最多支持10个输入参数,并支持函数返 \r\n");
      printf("回值显示.新增参数显示进制设置功能,新增进制转换功能.\r\n");
      printf("技术支持:www.openedv.com\r\n");
      printf("USMART有7个系统命令:\r\n");
      printf("?:      获取帮助信息\r\n");
      printf("help:   获取帮助信息\r\n");
      printf("list:   可用的函数列表\r\n\n");
      printf("id:     可用函数的ID列表\r\n\n");
      printf("hex:    参数16进制显示,后跟空格+数字即执行进制转换\r\n\n");
      printf("dec:    参数10进制显示,后跟空格+数字即执行进制转换\r\n\n");
      printf("runtime:1,开启函数运行计时;0,关闭函数运行计时;\r\n\n");
      printf("请按照程序编写格式输入函数名及参数并以回车键结束.\r\n");    
      printf("--------------------------ALIENTEK------------------------- \r\n");
      printf("指令失效\r\n");
      break;
    case 2:                                         //查询指令
      printf("\r\n");                               //换行
      printf("-------------------------函数清单--------------------------- \r\n");
      
      for(i = 0; i < s_iFuncNum; i++)
      {
        printf("%s\r\n", s_structNameTab[i].pFuncName); //循环打印出所有已注册的函数
      }
      
      printf("\r\n");                               //换行
      break;
    case 3:                                         //查询ID
      printf("\r\n");                               //换行
      printf("-------------------------函数 ID --------------------------- \r\n");
      for(i = 0; i < s_iFuncNum; i++)
      {
        GetFuncName((u8*)s_structNameTab[i].pFuncName, arrFuncName, &pnum, &rval);//得到本地函数名 
        printf("%s id is:\r\n0X%08X\r\n", arrFuncName, s_structNameTab[i].pFunc); //显示ID
      }
      printf("\r\n");                               //换行
      break;
    case 4:                                         //hex指令
      printf("\r\n");                               //换行
      GetAFuncPara(pStr, arrFuncName, &paraType);
      if(paraType == 0)                             //参数正常
      {
        sts = Str2Num(arrFuncName, &res);           //将字符串转为数字
        if(sts == 0)                                //转换成功
        {
          printf("HEX:0X%X\r\n", res);              //以16进制打印数字
        }
        else if(sts != 4)
        {
          return DBG_ASST_PARA_ERR;                 //参数错误.
        }
        else                                        //参数显示设定功能
        {
          printf("16进制参数显示!\r\n");
          s_iParaDispType = SP_TYPE_HEX;            //设置参数显示类型16进制
        }
      }
      else 
      {
        return DBG_ASST_PARA_ERR;                   //参数错误.
      }
      printf("\r\n");                               //换行 
      break;
    case 5:                                         //dec指令
      printf("\r\n");                               //换行
      GetAFuncPara(pStr, arrFuncName, &paraType);
      if(paraType == 0)                             //参数正常
      {
        sts = Str2Num(arrFuncName, &res);           //将字符串转为数字
        if(sts == 0)                                //转换成功
        {
          printf("DEC:%lu\r\n", res);               //以10进制打印数字
        }
        else if(sts != 4)
        {
          return DBG_ASST_PARA_ERR;                 //参数错误.
        }
        else                                        //参数显示设定功能
        {
          printf("10进制参数显示!\r\n");
          s_iParaDispType = SP_TYPE_DEC;            //设置参数显示类型10进制  
        }
      }
      else
      {
        return DBG_ASST_PARA_ERR;                   //参数错误.
      }  
      printf("\r\n");                               //换行 
      break;	 
    case 6:                                         //runtime指令,设置是否显示函数执行时间
      printf("\r\n");                               //换行
      GetAFuncPara(pStr, arrFuncName, &paraType);   //从pStr中得到该函数的参数
      if(paraType == 0)                             //参数正常 
      {         
        sts = Str2Num(arrFuncName, &res);           //将字符串转为数字
        if(sts == 0)                                //读取指定地址数据功能
        {
          s_iRunTimeFlag = res;                     //将读取出来的值赋给标记位
          if(s_iRunTimeFlag)
          {
            printf("Run Time Calculation ON\r\n");  //开启统计函数执行时间
          }
          else
          {
            printf("Run Time Calculation OFF\r\n"); //取消统计函数执行时间
          }
        }
        else
        {
          return DBG_ASST_PARA_ERR;                 //未带参数,或者参数错误	
        }
      }
      else
      {
        return DBG_ASST_PARA_ERR;                   //参数错误. 
      }
      printf("\r\n");                               //换行 
      break;    
    default:                                        //非法指令
      return DBG_ASST_FUNC_ERR;                     //错误的指令  
  }
  return 0;
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitDbgAsst
* 函数功能: 初始化
* 输入参数: void 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/	 
void InitDbgAsst(void)
{
  s_iParaDispType = SP_TYPE_HEX;                                  //十六进制显示参数
  
  s_iFuncNum = sizeof(s_structNameTab) / sizeof(StructNameTab);   //得到函数的数量
  
  InitProcString();                                               //初始化ProcString
}

/*********************************************************************************************************
* 函数名称: ReadAddr
* 函数功能: 读取指定地址的值
* 输入参数: addr-指定地址 
* 输出参数: void
* 返 回 值: 读取到指定地址的值
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/	 
u32 ReadAddr(u32 addr)
{
  return *(u32*)addr;   //通过强制转换把地址转换成指针类型
}

/*********************************************************************************************************
* 函数名称: WriteAddr
* 函数功能: 在指定地址写入指定的值
* 输入参数: addr-指定地址,val-要写入的值
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/	  
void WriteAddr(u32 addr, u32 val)
{
  *(u32*)addr = val;    //通过强制转换把地址转换成指针类型并对其赋值
}

/*********************************************************************************************************
* 函数名称: DbgAsstScan
* 函数功能: 扫描函数,该函数需要每隔一定时间被调用一次,用于处理从串口接收来的数据
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/	 
void DbgAsstScan(void)
{
  u8 sts;                                     //用于存放函数运行状态的标志位  
  u8 procRslt;                                //处理结果  
  u8 recData;                                 //接收的数据  
  i16 recCnt;                                 //接收到的数据计数器         
  u8  arrData[MAX_REC_DATA_CNT];              //数据缓冲区

  while(ReadUART(&recData, 1))                //当从串口读取到数据时
  {
    if(1 == CheckLineFeed(recData))           //检查是否携带换行符
    {     
      recCnt = GetRecData(arrData);           //获取接收到的数据
      arrData[recCnt] = '\0';                 //在末尾加入结束符
      
      sts = GetFuncInfo(arrData);             //得到函数各个信息
      if(sts == 0)                            //如果接受到的是函数
      {     
        ExeFunc();                            //执行函数 
      }     
      else      
      {       
        procRslt = ProcSysCmd(arrData);       //执行系统指令，接收函数执行结果
              
        if(procRslt)                          //分析指令错误情况
        {
          switch(procRslt)
          {
            case DBG_ASST_FUNC_ERR:
              printf("函数错误!\r\n");  
              break;	
            case DBG_ASST_PARA_ERR:
              printf("参数错误!\r\n");  
              break;  
            case DBG_ASST_PARA_OVER:
              printf("参数太多!\r\n");  
              break;  
            case DBG_ASST_FUNC_MISMATCH:
              printf("未找到匹配的函数!\r\n");  
              break;  
          }
        }
      }      
    }
  }
}

/*********************************************************************************************************
* 函数名称: SetParaType
* 函数功能: 设置参数的类型
* 输入参数: paraType：参数类型
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/
void SetParaType(u16 paraType)
{
  s_iParaType = paraType; //将传入的参数类型赋给s_iParaType
}

/*********************************************************************************************************
* 函数名称: GetParaType
* 函数功能: 得到参数的类型
* 输入参数: void
* 输出参数: void
* 返 回 值: s_iParaType：参数的类型
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/
u16 GetParaType(void)
{
  return(s_iParaType);    //返回参数的类型
}
