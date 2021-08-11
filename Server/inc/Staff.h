#ifndef __STAFF_H__
#define __STAFF_H__
#include <sqlite3.h>

#define IP "0.0.0.0"
#define PORT 8888
#define TXTLEN 512
#define ERRMSG(msg) printf("%s:%s:%d: %s \n",\
            __FILE__,__func__,__LINE__,msg)

//type
#define AREG    1   //注册     struct
#define ALOAD   2   //登录     struct
#define AFINDUSER 3   //查询用户信息  string colx = valx
#define AFINDATTD 4   //查询打卡信息  string colx = valx
#define ACHANGEUSER 5   //修改用户信息  struct
#define ACHANGEATTD 6   //修改打卡信息  struct
#define ADELUSER 7  //删除用户  string colx = valx
#define AACK    8   //反馈     只读type 
#define AERR    9   //错误     只读type
#define ACLOCKIN 10  //打卡     只读type
#define AEXIT   11  //客户端退出

struct order
{
	unsigned int num; // txt段有效字节数
	char type;  
	char txt[TXTLEN]; 
};//__attribute__((packed))

typedef struct STAFF_INFO 
{
    int id;
    char code[32];
    char name[32];
    int level; 
    float salary;
    int state;  //0-load  | 1-unload 
}Staff;

#define INFO_COL 6

typedef struct STAFF_ATTENDANCE
{
    int id;
    char time[64];  // 年 月 日 时 分 秒
    int clockin;  //打卡（0-上班 | 1-下班）
}Attendance;

#define ATTEND_COL 3

int order2struct(void *retval,int fds);

int Initial_fun(sqlite3 *db);

int Register_fun(sqlite3 *db,struct order *sndbuf,int fds);

int Login_fun(sqlite3 *db,Staff *sp,int fds);

int Search_fun(sqlite3* db,struct order *sndbuf,int fds);

int Change_fun(sqlite3* db,struct order *sndbuf,int fds);

int Delete_fun(sqlite3* db,struct order *sndbuf,int fds);

int Clockin_fun(sqlite3* db,struct order *sndbuf,int fds);

#endif