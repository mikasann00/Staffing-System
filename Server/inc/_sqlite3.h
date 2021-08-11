#ifndef __SQLITE3_H__
#define __SQLITE3_H__
#include <sqlite3.h>
#include "Staff.h"

#define SQLLEN 256
#define ERR_SQ(msg) do{printf(msg); \
    printf(":%s %s %d %s\n",__FILE__,__func__,__LINE__,sqlite3_errmsg(db));\
    }while(0)

//STAFF_INFO STAFF_ATTENDANCE
#define EITEM   -1  //对象不存在
#define ECODE   -2  //密码错误
#define EUPDATE -3  //更新失败
#define ELOADED -4  //账号已登录
#define EADD    -5  //添加失败
#define EDEL    -6  //删除失败
#define ENAME   -7  //用户名错误或已存在
#define ECONNECT -8 //通讯错误
#define ECLOSE  -9  //对方关闭
#define EFIND   -10 //查询失败
/************func*****************/

void str2num(char *str,float *folret,int *intret);

void struct_show(void *data,int flag);

void tb_str2struct(char **str,void *retval,int flag);

void tb_err_print(int errno);

int tb_check(char *tableN);

int tb_isexist(sqlite3* db,char *tableN);

int tb_show(sqlite3* db);

int tb_find(sqlite3* db,char *tableN, char *value,char *colN,
            int mode,int *rows,void *retval,char ***retarray);
int tb_Search(sqlite3* db,char *tableN,char *where,
                int *rows,void *retval,char ***retarray);

int tb_add(sqlite3* db,char *tableN,void *data);

int tb_mul_add(sqlite3* db,char *tableN,char *path);

int tb_delete(sqlite3 *db,char *tableN,char *condition);

int tb_modify(sqlite3* db,char *tableN,int id,void *data);
int tb_update(sqlite3* db,char *tableN,char *where,char *set);

int tb_idcheck(sqlite3 *db,int id);






#endif