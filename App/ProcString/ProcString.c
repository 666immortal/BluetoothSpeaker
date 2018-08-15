﻿/*********************************************************************************************************
* 模块名称: ProcString.c
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
#include "ProcString.h"
#include "DbgAsst.h"
#include "DataType.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static u8 s_arrParaLenTbl[MAX_PARA_NUM];    //参数长度存放表，数字的长度默认为四个字节

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static u32 Power(u8 m, u8 n);               //计算m^n函数
static u8  GetNextChar(u8* pStr);           //获取下一个非空格的字符

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: Power
* 函数功能: 计算m^n函数
* 输入参数: m，n
* 输出参数: void
* 返 回 值: m^n次方
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/   
static u32 Power(u8 m, u8 n)
{
  u32 rslt = 1;         //定义用来存放结果的变量      
  
  while(n--)            //随着每次循环，n递减，直至为0
  {
    rslt *= m;          //循环n次，相当于n个m相乘
  }
  
  return rslt;          //返回m的n次幂的值
} 

/*********************************************************************************************************
* 函数名称: GetNextChar
* 函数功能: 获取下一个非空格的字符（当中间有很多空格的时候，此函数直接忽略空格，找到空格之后的第一个字符）
* 输入参数: pStr:字符串指针
* 输出参数: void
* 返 回 值: 下一个字符
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/    
static u8 GetNextChar(u8* pStr)
{
  pStr++;                               //指针指向下一个字符
  
  while(*pStr == ' ' && *pStr != '\0')  //如果指针是空格或者结束符
  {
    pStr++;                             //指针指向下一个字符 
  }  
  
  return(*pStr);                        //返回下一个非空格的字符
} 

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitProcString
* 函数功能: 初始化ProcString模块
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/
void InitProcString(void)
{
  i16 i;                              //循环计数器
  
  for(i = 0; i < MAX_PARA_NUM; i++)   
  {
    s_arrParaLenTbl[i] = 0;           //循环将参数长度存放表清零
  }
}

/*********************************************************************************************************
* 函数名称: StrCmp
* 函数功能: 对比字符串pStr1和pStr2 
* 输入参数: pStr1:字符串1指针,pStr2:字符串2指针
* 输出参数: void
* 返 回 值: 0，相等;1，不相等
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/
u8 StrCmp(u8* pStr1, u8* pStr2)
{
  while(1)
  {
    if(*pStr1 != *pStr2)//两个字符串不相等
    {
      return 1;         //返回1，表示不相等
    }
    
    if(*pStr1 == '\0')
    {
      break;            //对比完成，跳出出循环
    }
    
    pStr1++;            //pStr1指针加1
    pStr2++;            //pStr2指针加1
  }
  
  return 0;             //返回0，表示两个字符串相等
}

/*********************************************************************************************************
* 函数名称: StrCpy
* 函数功能: 把pStr1的内容拷贝到pStr2 
* 输入参数: pStr1:字符串1指针,pStr2:字符串2指针
* 输出参数: pStr1:字符串1指针,pStr2:字符串2指针
* 返 回 值: void
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/
void StrCpy(u8* pStr1, u8* pStr2)
{
  while(1)
  {
    *pStr2 = *pStr1;    //拷贝pStr1的内容到pStr2 
    
    if(*pStr1 == '\0')  //如果读到了结束符
    {
      break;            //拷贝完成
    }
    
    pStr1++;            //pStr1指针加1
    pStr2++;            //pStr2指针加1
  }
}

/*********************************************************************************************************
* 函数名称: StrLen
* 函数功能: 得到字符串的长度(字节)
* 输入参数: pStr:字符串指针
* 输出参数: void
* 返 回 值: 字符串的长度
* 创建日期: 2018年03月01日
* 注    意:
*********************************************************************************************************/   
u8 StrLen(u8* pStr)
{
  u8 len = 0;           //用于存放字符串的长度
  
  while(1)
  { 
    if(*pStr == '\0')   //如果读到了结束符
    {
      break;            //字符串已读完
    }  
    
    len++;              //长度加1
    pStr++;             //指针加1
  }
  
  return len;           //返回字符串的长度
}

/*********************************************************************************************************
* 函数名称: Str2Num
* 函数功能: 将字符串转为数字
* 输入参数: pStr:字符串指针，pRslt:转换完的结果存放的地址
* 输出参数: void
* 返 回 值: 0-成功转换完成，1-数据格式错误，2-16进制位数为0，3-起始格式错误，4-十进制位数为0
* 创建日期: 2018年03月01日
* 注    意: 支持16进制转换,但是16进制字母必须是大写的,且格式为以0X开头的,不支持负数
*********************************************************************************************************/   
u8 Str2Num(u8* pStr, u32* pRslt)
{
  u32 t;                                          //用于存放转换的结果
  u8 digitNum = 0;                                //数字的位数
  u8 *p;                                          //指向字符串
  u8 hexDecFlag = 10;                             //10-十进制，16-十六进制，默认为十进制数据
                          
  p = pStr;                                       //定位到字符串开始的地址
  *pRslt = 0;                                     //清零
  
  while(1)
  {
    if((*p <= '9' && *p >= '0') || (*p <= 'F' && *p >= 'A') || (*p == 'X' && digitNum == 1))//参数合法
    {
      if(*p >= 'A')
      {
        hexDecFlag = 16;                          //字符串中存在字母,为16进制格式
      }                         
                            
      digitNum++;                                 //位数增加
    }                       
    else if(*p == '\0')                       
    {                       
      break;                                      //碰到结束符,退出while(1)循环
    }                       
    else                        
    {                       
      return 1;                                   //不全是十进制或者16进制数据，直接退出
    }                       
                            
    p++;                                          //指针加1    
  }                         
                          
  p = pStr;                                       //重新定位到字符串开始的地址
                          
  if(hexDecFlag == 16)                            //16进制数据
  {               
    if(digitNum < 3)                
    {               
      //位数小于3，直接退出。因为0X就占了2个,如果0X后面不跟数据,则该数据非法  
      return 2;                           
    }                 
    
    if(*p == '0' && (*(p + 1) == 'X'))            //必须以'0X'开头.
    {       
      p += 2;                                     //偏移到数据起始地址
      digitNum -= 2;                              //减去偏移量
    }                       
    else                        
    {                       
      return 3;                                   //起始头的格式不对
    }                       
  }                       
                          
  if(digitNum == 0)                       
  {                       
    return 4;                                     //位数为0，直接退出	  
  }                       
                          
  while(1)                        
  {                       
    if(digitNum)                                  //如果数字的位数还不等于0
    {               
      digitNum--;                                 //数字位数减1
    }  
    
    if(*p <= '9' && *p >= '0')                    //数字为0到9
    {
      t = *p - '0';                               //得到数字的值
    }                         
    else                        
    {                       
      t = *p - 'A' + 10;                          //得到A-F对应的值 
    }  
    
    *pRslt += t * Power(hexDecFlag, digitNum);    //计算值
    p++;                                          //指针加1
    
    if(*p == '\0')
    {
      break;                                      //数据都查完了 
    }                           
  }                       
                          
  return 0;                                       //转换成功
}               

/*********************************************************************************************************
* 函数名称: GetParaPos
* 函数功能: 得到指定参数的起始地址
* 输入参数: num:第num个参数,范围0-9
* 输出参数: void
* 返 回 值: 该参数的起始地址
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/    
u8 GetParaPos(u8 num)
{
  u8 temp = 0;                  //用来存放指定参数的起始地址
  u8 i;                         //循环计数器
  
  for(i = 0; i < num; i++)      
  {
    temp += s_arrParaLenTbl[i]; //加上新的偏移量
  }
  
  return temp;                  //返回参数的起始地址
}

/*********************************************************************************************************
* 函数名称: GetCmdName
* 函数功能: 从pStr中得到指令名,并返回指令长度
* 输入参数: pStr-源字符串指针，pCmdName-指令名，pCmdNameLen-指令名长度，maxLen-最大长度(做限制,指令不可能太长)
* 输出参数: void
* 返 回 值: 0,成功;其他,失败
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/    
u8 GetCmdName(u8* pStr, u8* pCmdName, u8* pCmdNameLen, u8 maxLen)
{
  *pCmdNameLen = 0;                     //清零
  
  while(*pStr != ' ' && *pStr != '\0')  //找到空格或者结束符则认为结束
  {
    *pCmdName = *pStr;                  //将pStr的值赋到pCmdName中
    pStr++;                             //pStr指针加1
    pCmdName++;                         //pCmdName指针加1
    (*pCmdNameLen)++;                   //统计命令长度
    
    if(*pCmdNameLen >= maxLen)          //如果指令的长度大于最大长度
    {
      return 1;                         //错误的指令  
    }  
  }
  
  *pCmdName = '\0';                     //加入结束符
  
  return 0;                             //正常返回
}

/*********************************************************************************************************
* 函数名称: GetFuncName
* 函数功能: 从pStr中得到函数名，参数个数以及是否需要返回值
* 输入参数: pStr:源字符串指针,pFuncName:获取到的函数名字指针,pFuncParaNum:函数的参数个数
*           pDispReturnValFlag:是否需要显示返回值(0,不需要;1,需要)
* 输出参数: void
* 返 回 值: 0,成功;其他,错误代码
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/    
u8 GetFuncName(u8* pStr, u8* pFuncName, u8* pFuncParaNum, u8* pDispReturnValFlag)
{
  u8 res;                                           //用于记录函数执行结果
  u8 fover = 0;                                     //括号层数
  u8* pStrTemp;                                     //用来指向pStr
  u8 offset = 0;                                    //记录函数名之前的字符数
  u8 paraNum = 0;                                   //用来记录参数的个数
  u8 temp = 1;                                      //用来标志参数是否有效
  u8 arrFuncName[6];                                //记录字符串 void + X + '/0'
  u8 firstParaLenCnt = 0;                           //第一个参数的长度计数器
  u8 paraCnt = 0;                                   //参数计数器
  u8 nchar;                                         //表示下一个字符
  
  //判断函数是否有返回值
  pStrTemp = pStr;                                  //pStrTemp指针指向pStr
  while(*pStrTemp != '\0')                          //没有结束
  {
    if(*pStrTemp != ' ' && (paraCnt & 0X7F) < 5)    //最多记录5个字符
    { 
      if(paraCnt == 0)                              //如果参数数量等于0
      {
        paraCnt |= 0X80;                            //最高位置1,标记开始接收返回值类型  
      }
      
      if(((paraCnt & 0x7f) == 4) && (*pStrTemp != '*'))//最后一个字符,必须是* 
      {
        break;                                       
      }  
      
      arrFuncName[paraCnt & 0x7f] = *pStrTemp;      //记录函数的返回值类型
      paraCnt++;                                    //参数数量加1
    }
    else if(paraCnt == 0X85)                        //参数量超过5
    {
      break;                                        //跳出循环
    }  
    
    pStrTemp++;                                     //pStrTemp指针加1
  }
  
  if(paraCnt)                                       //接收完了
  {
    arrFuncName[paraCnt & 0x7f] = '\0';             //加入结束符
    if(StrCmp(arrFuncName, "void") == 0)            //比较字符串和“void”
    {
      *pDispReturnValFlag = 0;                      //不需要返回值  
    }  
    else 
    {
      *pDispReturnValFlag = 1;                      //需要返回值  
    }  
    paraCnt = 0;                                    //将参数数量记为0
  }
  
  res = 0;                                          //函数执行结果标志设置为0
  pStrTemp = pStr;                                  //pStrTemp指针重新指向pStr
  //寻找到函数名的真正起始位置
  while(*pStrTemp != '(' && *pStrTemp != '\0')      
  {  
    pStrTemp++;                                     //pStrTemp加1，指向下一个字符
    res++;                                          //记录读过的字符数                                   
    if(*pStrTemp == ' ' || *pStrTemp == '*')
    {
      nchar = GetNextChar(pStrTemp);                //获取下一个非空格的字符
      if(nchar != '(' && nchar != '*')
      {
        offset = res;                               //将函数名之前的字符数赋给offset，跳过空格和*号  
      }  
    }
  }
  pStrTemp = pStr;                                  //pStrTemp重新指向pStr
  if(offset)
  {
    pStrTemp += offset + 1;                         //跳到函数名开始的地方
  }    
  res = 0;                                          //函数执行结果标志设置为0
  nchar = 0;                                        //是否正在字符串里面的标志,0，不在字符串;1，在字符串;
  //开始读取参数
  while(1)
  {
    if(*pStrTemp == 0)
    {
      res = DBG_ASST_FUNC_ERR;                      //函数错误
      break;
    }
    else if(*pStrTemp == '(' && nchar == 0)
    {
      fover++;                                      //括号层数加1
    }
    else if(*pStrTemp == ')' && nchar == 0)         
    {
      if(fover)                                     //如果fover层数不为0
      {
        fover--;                                    //括号层数减1
      }  
      else 
      {
        res = DBG_ASST_FUNC_ERR;                    //错误结束,没收到'('
      }  
      if(fover == 0)                                //括号层数为0时
      {
        break;                                      //到末尾了,退出  
      }
    }
    else if(*pStrTemp == '"')
    {
      nchar = !nchar;                               //是否在字符串里面的标志翻转
    }
    
    if(fover == 0)                                  //括号层数为0，函数名还没接收完
    {
      if(*pStrTemp != ' ')                          //空格不属于函数名
      {
        *pFuncName = *pStrTemp;                     //得到函数名
        pFuncName++;                                //pFuncName指针加1
      }
    }
    else                                            //已经接受完了函数名了，开始接受括号里的参数
    {
      if(*pStrTemp == ',')                          //当读到‘，’时
      {
        temp = 1;                                   //使能增加一个参数
        paraCnt++;                                  //参数数量加1
      }
      else if(*pStrTemp != ' ' && *pStrTemp != '(')
      {
        if(paraCnt == 0 && firstParaLenCnt < 5)     //当第一个参数来时,为了避免统计void类型的参数,必须做判断
        {
          arrFuncName[firstParaLenCnt] = *pStrTemp; //记录参数特征.
          firstParaLenCnt++;                        //首参数指针向后移
        }
        temp++;                                     //得到有效参数(非空格)
      }
      if(fover == 1 && temp == 2)
      {
        temp++;                                     //防止重复增加
        paraNum++;                                  //参数增加一个
      }
    }
    pStrTemp++;                                     //pStrTemp指针加1，指向下一个字符
  }   
  
  if(paraNum == 1)                                  //只有1个参数.
  {
    arrFuncName[firstParaLenCnt] = '\0';            //加入结束符
    
    if(StrCmp(arrFuncName, "void") == 0)            //将字符串和“void”进行比较
    {
      paraNum = 0;                                  //参数为void,表示没有参数  
    }                     
  }                     
                        
  *pFuncParaNum = paraNum;                          //记录参数个数
  *pFuncName = '\0';                                //加入结束符
                        
  return res;                                       //返回执行结果
}

/*********************************************************************************************************
* 函数名称: GetAFuncPara
* 函数功能: 从pStr中得到函数的一个参数
* 输入参数: pStr:源字符串指针, pParaStr:参数字符串指针, pParaType:参数类型 0，数字;1，字符串;0XFF，参数错误
* 输出参数: void
* 返 回 值: 0,已经无参数了;其他,下一个参数的偏移量
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/    
u8 GetAFuncPara(u8* pStr, u8* pParaStr, u16* pParaType)
{
  u8  i      = 0;                           //循环计数器
  u8  enout  = 0;                           //暂缓立即退出标志
  u16 type   = 0;                           //默认是数字
  u8  string = 0;                           //标记str是否正在读
  
  while(1)
  { 
    if(*pStr == ',' && string == 0)
    {
      enout = 1;                            //暂缓立即退出,目的是寻找下一个参数的起始地址  
    }  
    
    if((*pStr == ')' || *pStr == '\0') && string == 0)
    {
      break;                                //立即退出标识符  
    }  
    
    if(type == 0)                           //默认是数字类型
    {
      //数字串检测
      if((*pStr >= '0' && *pStr <= '9') || (*pStr >= 'a' && *pStr <= 'f') 
        || (*pStr >= 'A' && *pStr <= 'F') || *pStr == 'X'|| *pStr == 'x')
      {
        if(enout)
        {
          break;                            //找到了下一个参数,直接退出  
        } 
        
        if(*pStr >= 'a')
        {
          *pParaStr = *pStr - 0X20;         //小写转换为大写  
        }          
        else 
        {
          *pParaStr = *pStr;                //小写或者数字保持不变
        }                 
                        
        pParaStr++;               
      }               
      else if(*pStr == '"')                 //找到字符串的开始标志
      {               
        if(enout)               
        {               
          break;                            //找到‘,’后才找到‘"’,认为结束了  
        }               
                        
        type   = 1;               
        string = 1;                         //登记string，正在读了        
      }
      else if(*pStr != ' ' && *pStr != ',') //发现非法字符
      {
        type = 0XFF;                        //参数错误
        break;                              //跳出循环
      }
    }
    else                                    //如果是字符串类型
    {
      if(*pStr == '"')                
      {               
        string = 0;                 
      }                 
                      
      if(enout)               
      {                 
        break;                              //找到了下一个参数,跳出循环
      }                       
      if(string)                            //字符串正在读
      {               
        if(*pStr == '\\')                   //遇到转义符(不复制转义符)
        {                 
          pStr++;                           //偏移到转义符后面的字符,不管什么字符,直接COPY
          i++;                
        }      
        
        *pParaStr = *pStr;                  //小写或者数字保持不变
        pParaStr++;                         //pParaStr指针加1
      }                 
    }               
    
    i++;                                    //参数长度加1
    pStr++;                                 //pStr指针加1
  } 
  
  *pParaStr  = '\0';                        //加入结束符
  *pParaType = type;                        //返回参数类型
                  
  return i;                                 //返回参数长度，也就是下一个参数的偏移量
}

/*********************************************************************************************************
* 函数名称: GetAllFuncPara
* 函数功能: 从pStr中得到函数所有参数
* 输入参数: pStr:源字符串
* 输出参数: paraNum:参数个数.0表示无参数 void类型
* 返 回 值: 0,成功;其他,错误代码
* 创建日期: 2018年03月01日
* 注    意: 
*********************************************************************************************************/    
u8 GetAllFuncPara(u8* pStr, u8* pParaNum)
{
  u8  i;                                            //循环计数器
  u8  sts;                                          //用于存放函数运行状态的标志位                         
  u16 paraType;                                     //参数类型  
  u32 rslt;                                         //转换后的数字结果
  u8  paraNum = 0;                                  //参数个数
  u8  len;                                          //用于存放字符串的长度
  u8  arrParaStr[PARM_LEN + 1];                     //字节长度的缓存,最多可以存放PARM_LEN个字符的字符串
  
  for(i = 0; i < MAX_PARA_NUM; i++)
  {
    s_arrParaLenTbl[i]=0;                           //清空参数长度表
  }
  
  while(*pStr != '(')                               //偏移到参数开始的地方
  {
    pStr++;	
    
    if(*pStr == '\0')
    {
      return DBG_ASST_FUNC_ERR;                     //遇到结束符了  
    }  
  }
  
  pStr++;                                           //偏移到"("之后的第一个字节，例如LEDSetTwo(1,2);的“1”
  
  while(1)
  {
    i = GetAFuncPara(pStr, arrParaStr, &paraType);  //得到第1个参数、第2个参数、...，返回值为下一个参数的偏移量  
    pStr += i;                                      //字符串指针偏移到下一个参数
    
    switch(paraType)
    {
      case 0:	//数字
        if(arrParaStr[0] != '\0')                   //接收到的参数有效
        {
          sts = Str2Num(arrParaStr, &rslt);         //将字符串转为数字
        
          if(sts)
          {
            return DBG_ASST_PARA_ERR;               //参数错误  
          }  
          //将g_arrPara偏移到下一个，强制转换成指针类型，并用来记录参数转换成功的结果，数字为四字节对齐 
          *(u32*)(g_arrPara + GetParaPos(paraNum)) = rslt;                 
          
          paraType = GetParaType();                 //得到参数的类型
          paraType &= ~(1 << paraNum);              //按位记录参数的类型
          SetParaType(paraType);                    //设置参数的类型
          
          s_arrParaLenTbl[paraNum] = 4;             //该参数的长度为4，数字长度默认占4个字节  
          paraNum++;                                //参数数量加1
          
          if(paraNum > MAX_PARA_NUM)                //如果参数的数量大于最大范围
          {
            return DBG_ASST_PARA_OVER;              //参数太多  
          }  
        }
        break;
      case 1://字符串
        len = StrLen(arrParaStr) + 1;               //包含了结束符'\0'
        //拷贝arrParaStr的数据到g_arrPara
        StrCpy(arrParaStr, &g_arrPara[GetParaPos(paraNum)]);
      
        paraType = GetParaType();                   //得到参数的类型
        paraType |= 1 << paraNum;                   //标记字符串 
        SetParaType(paraType);                      //设置参数的类型
      
        s_arrParaLenTbl[paraNum] = len;             //该参数的长度为len  
        paraNum++;                                  //参数数量加1
      
        if(paraNum > MAX_PARA_NUM)                  //如果参数的数量大于最大范围
        {
          return DBG_ASST_PARA_OVER;                //参数太多  
        }  
        
        break;
      case 0XFF://错误
        return DBG_ASST_PARA_ERR;                   //参数错误  
    }
    
    if(*pStr == ')' || *pStr == '\0')
    {
      break;                                        //检查结束标志
    }
  }
  
  *pParaNum = paraNum;                              //记录参数的个数
  
  return DBG_ASST_OK;                               //正确得到了参数
}
