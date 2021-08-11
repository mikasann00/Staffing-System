#ifndef __CILENT_H__
#define __CILENT_H__
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

#define INPUTCHECK(format,num)		\
			do{	int tpret;\
				do{\
                    tpret = scanf(format, &num);\
                    while (getchar() != 10);\
                    if(!tpret){\
                        printf("错误输入\n");\
                    }\
                }while(!tpret);\
			}while(0)

//INPUTCHECK("%d",a);

void LoginFun(int fds,Staff *retval);

void RegisterFun(int fds);

void SearchUserFun(int fds,Staff *curuser);

void SearchAttendFun(int fds,Staff *curuser);

void ChangeFun(int fds,Staff *curuser);

void DeleteFun(int fds,Staff *curuser);

void Clockinfun(int fds,Staff *curuser);

int menu(void);
int adminmenu(void);
int usermenu(void);
int modifymenu(int level);

int admincontrol(int fds,Staff *sp);
int usercontrol(int fds,Staff *sp);

#endif
