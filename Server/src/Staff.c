#include "Staff.h"
#include "_tcp.h"
#include "_sqlite3.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*获取时间
函数名称： gettime
函数功能： 获取时间
函数输入： char *str：
函数输出：
*/
void gettime(char *str)
{
    time_t t = time(NULL);
    struct tm *time = localtime(&t);
    sprintf(str,"%2d月%2d日 %02d:%02d",time->tm_mon + 1,time->tm_mday,
                        time->tm_hour,time->tm_min);
}


/*接收tcp包并转结构体
函数名称： order2struct
函数功能： 接收tcp包并转结构体
函数输入：int fds：
        void *retval:
函数输出：1 feedback package | 0 sucess  | <0-error code
*/
int order2struct(void *retval,int fds)
{
    struct order op;
    int ret = TcpRcv(fds,&op);
    if(ret == -1)
        return ECLOSE;
    else if(ret == -2)
        return ECONNECT;
    switch (op.type){
    case AREG:
    case ALOAD:
    case ACHANGEUSER:
        memcpy(retval,op.txt,op.num);
        break;
    case AACK:
        return 1;        
    default: break;
    }
    return 0;
}

/*拆分colx=valx
函数名称： txt2val
函数功能： 拆分colx=valx
函数输入：char *txt：
        char sep:拆分字符
        char *col:
        char *val:
函数输出：int ret 0 sucess/<0-error code
*/
void txt2val(char *txt,char sep,char *left,char *right)
{
    int flag = 1;
    while (*txt != '\0'){
        if(*txt == sep){
            flag = 0;
            txt++;
            continue;
        }
        if(flag){
            *left++ = *txt++;
        }else{
            *right++ = *txt++;
        }
    }   
}

////////////////////////////////////////////////////////////
/*初始化表格
函数名称：Initial_fun
函数功能：初始化表格，创建dic并导入词典，创建user并更新用户状态
函数输入：sqlite3 *db：
函数输出：int ret 0 sucess/<0-error code
*/
int Initial_fun(sqlite3 *db)
{
    char userpath[]="./user.txt";    //用户路径
    char attendpath[]="./attendance.txt"; //打卡记录路径
    char sql[SQLLEN]="";
    //检查用户表格  STAFF_INFO 
    if(!tb_isexist(db,"STAFF_INFO")){
        printf("STAFF_INFO不存在,准备创建\n");
        strcpy(sql,"create table STAFF_INFO (id int primary key,code char,name char,level int,salary real,state int)");
        if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
            ERR_SQ("sqlite3 create STAFF_INFO");
            return EADD;
        }
        printf("Create table STAFF_INFO done!\n");
        //导入词典 
        int ret = tb_mul_add(db,"STAFF_INFO",userpath);
        if(ret){
            ERRMSG("tb_mul_add");
            return ret;
        }
        printf("add STAFF_INFO done!\n");
    }
    //检查打卡记录表格 STAFF_ATTENDANCE
    if(!tb_isexist(db,"STAFF_ATTENDANCE")){
        memset(sql,0,sizeof(sql));
        printf("STAFF_ATTENDANCE不存在,准备创建\n");
        strcpy(sql,"create table STAFF_ATTENDANCE (id int,time char,clockin int)");
        if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
            ERR_SQ("sqlite3 create STAFF_ATTENDANCE");
            return EADD;
        }
        printf("Create table STAFF_ATTENDANCE done!\n");
        //导入词典 
        int ret = tb_mul_add(db,"STAFF_ATTENDANCE",attendpath);
        if(ret){
            ERRMSG("tb_mul_add");
            return ret;
        }
        printf("add STAFF_ATTENDANCE done!\n");
    } 
    return 0;
}

/*注册
函数名称： Register_fun
函数功能： 注册
函数输入： struct order *sndbuf
         sqlite3 *db 
         int fds
函数输出：0-success | <0-error code
*/
int Register_fun(sqlite3 *db,struct order *sndbuf,int fds)
{
    int ret;
    Staff tmp;
    memcpy(&tmp,sndbuf->txt,sndbuf->num);
    do{
        ret = tb_idcheck(db,tmp.id);
        if(ret){
            TcpSnd(fds,AERR,"id重复",0);
            ret = order2struct(&tmp,fds);
            if(ret < 0){  //通讯错误           
                return ECONNECT;
            }else if(ret == 1){
                return 0; //客户端退出该功能
            }else{
                ret = 1; //继续循环对比
            }
        }
    }while (ret);    
    
    if(TcpSnd(fds,AACK,"id可用",0))
        return ECONNECT;
    //接收完整注册包
    ret = order2struct(&tmp,fds);
    if(ret)
        return ret;
    
    //添加用户
    ret = tb_add(db,"STAFF_INFO",&tmp);
    if(ret)
        return ret;
    TcpSnd(fds,AACK,"Register success",0); //feedback
    return 0;
}

/*登录
函数名称：LOGIN
函数功能：核对用户名和密码，检查用户登录状态，成功则改变登录状态,同时更新sp至最新数据
函数输入：  sqlite3* db
          Staff *sp
          int fds
函数输出：0-success | <0-error code
*/
int Login_fun(sqlite3* db,Staff *sp,int fds)
{
    int ret = tb_idcheck(db,sp->id);
    if(!ret){
        TcpSnd(fds,AERR,"对象不存在",0);
        return EITEM;
    }

    Staff tmp,errtmp;
    memcpy(&errtmp,sp,sizeof(Staff));

    char findtxt[64]="";
    sprintf(findtxt,"id = %d",sp->id);

    //tb_find(db,"STAFF_INFO",findtxt,"id",1,NULL,&tmp,NULL);
    tb_Search(db,"STAFF_INFO",findtxt,NULL,&tmp,NULL);
    //密码校验
    int count = 3;
    while (count){
        count--;
        if(!strcmp(errtmp.code,tmp.code))
            break;
        
        char errtxt[128]="";
        sprintf(errtxt,"用户名错误或密码错误,剩余尝试机会%d次",count);
        TcpSnd(fds,AERR,errtxt,0);
        ret = order2struct(&errtmp,fds);
        if(ret)
            return ret;
    }    
    if(count <= 0){
        TcpSnd(fds,AERR,"error loaded",0);
        return ECODE;
    }
    tmp.state = 0;
    ret = tb_modify(db,"STAFF_INFO",tmp.id,&tmp);
    if(ret)
        return ret;
    TcpSnd(fds,AACK,(char *)&tmp,sizeof(tmp)); //feedback
    memcpy(sp,&tmp,sizeof(Staff)); //更新sp至最新数据
    return 0;
}


/*查询
函数名称：Search_fun
函数功能：查询信息
函数输入：  sqlite3* db
          struct order *sndbuf
          int fds
函数输出：0-success | <0-error code
*/
int Search_fun(sqlite3* db,struct order *sndbuf,int fds)
{
    int ret;
    int rows;
    char **result;
    if(sndbuf->type == AFINDUSER){
        ret = tb_Search(db,"STAFF_INFO",sndbuf->txt,
                &rows,NULL,&result);
    }else if(sndbuf->type == AFINDATTD){
        ret = tb_Search(db,"STAFF_ATTENDANCE",sndbuf->txt,
                &rows,NULL,&result);
    }
    if(ret){
        TcpSnd(fds,AERR,"无匹配项",0);
        return ret;
    }
    char txt[TXTLEN] = "";
    if(sndbuf->type == AFINDUSER){
        for (int i = 0; i <= rows; i++){
            for (int j = 0; j < INFO_COL; j++){
                strcat(txt,result[i*INFO_COL+j]);
                strcat(txt,"\t");
            }
            strcat(txt,"\n");
        }        
    }else if(sndbuf->type == AFINDATTD){
        for (int i = 0; i <= rows; i++){
            for (int j = 0; j < ATTEND_COL; j++){
                strcat(txt,result[i*ATTEND_COL+j]);
                strcat(txt,"\t");
            }
            strcat(txt,"\n");
        } 
    }
    sqlite3_free_table(result);
    TcpSnd(fds,AACK,txt,0);
    return 0;
}

/*修改
函数名称： Change_fun
函数功能：查询信息
函数输入：  sqlite3* db
          struct order *sndbuf
          int fds
函数输出：0-success | <0-error code
*/
int Change_fun(sqlite3* db,struct order *sndbuf,int fds)
{
    int ret;
    struct order rcvbuf;
    char where[128]="";
    char set[128]="";
    //检测修改类型和id，
    ret = tb_idcheck(db,atoi(sndbuf->txt));
    if(!ret){
        TcpSnd(fds,AERR,"无匹配项",0);
        return ret;
    }
    TcpSnd(fds,AACK,"有匹配项",0);
    TcpRcv(fds,&rcvbuf);
    txt2val(rcvbuf.txt,'\n',set,where);
    ret = tb_update(db,"STAFF_INFO",where,set);    
    if(ret){
        TcpSnd(fds,AERR,"更新失败",0);
        return ret;
    }
    TcpSnd(fds,AACK,"modify success",0); //feedback
    return 0;
}

/*删除
函数名称： Delete_fun
函数功能：查询信息
函数输入：  sqlite3* db
          struct order *sndbuf
          int fds
函数输出：0-success | <0-error code
*/
int Delete_fun(sqlite3* db,struct order *sndbuf,int fds)
{
    int ret = tb_delete(db,"STAFF_INFO",sndbuf->txt);
    if(ret){
        TcpSnd(fds,AERR,"对象不存在",0);
        return ret;
    }
    ret = tb_delete(db,"STAFF_ATTENDANCE",sndbuf->txt);
    if(ret){
        return ret;
    }
    TcpSnd(fds,AACK,"操作完毕",0);
    return 0;
}

/*打卡
函数名称： Clockin_fun
函数功能： 打卡
函数输入：  sqlite3* db
          struct order *sndbuf
          int fds
函数输出：0-success | <0-error code
*/
int Clockin_fun(sqlite3* db,struct order *sndbuf,int fds)
{
    int ret;
    Attendance ap;
    memcpy(&ap,sndbuf->txt,sndbuf->num);
    gettime(ap.time);
    ap.clockin = 0; //废弃该属性
    ret = tb_add(db,"STAFF_ATTENDANCE",&ap);
    if(ret){
        TcpSnd(fds,AERR,"打卡失败",0);
        return ret;
    }
    TcpSnd(fds,AACK,"打卡成功",0);
    return 0;
}

