#include "_sqlite3.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/*字符串转数字
函数名称： str2num
函数功能： 字符串转数字
函数输入： char *str:  
         float *folret: NULL无返回
         int *intret: NULL无返回
函数输出： 无
*/
void str2num(char *str,float *folret,int *intret)
{
    int flag = 0,div = 1;
    float folval = 0;
    if(str == NULL)
        return;
    while (*str != '\0'){    
        if (*str == '.') 
            flag = 1;
        else if(flag)
            div *= 10;
        if(*str >= '0' && *str <= '9'){
            folval = folval*10 + *str - '0';
        } 
        str++;
    }
    folval /= div;
    if(folret != NULL)
        *folret = folval;
    if(intret != NULL)
        *intret = (int)folval;
}

/*打印结构体数据
函数名称： struct_show
函数功能： 打印结构体数据
函数输入： void *data: 结构体
         int flag: 1-STAFF_INFO | 2-STAFF_ATTENDANCE
函数输出： 无
*/
void struct_show(void *data,int flag)
{
    if(flag == 1){
        Staff *sp = (Staff *)data;
        printf("id %d\tcode %s\tname %s\tlevel %d\tsalary %.2f\tstate %d\n",
            sp->id,sp->code,sp->name,sp->level,sp->salary,sp->state);
    }else if(flag == 2 ){
        Attendance *ap = (Attendance *)data; 
        printf("id %d\ttime %s\tclockin %d\n",ap->id,ap->time,ap->clockin);
    }
}


/*数据库文本转结构体(单个)
函数名称： tb_str2struct
函数功能： 数据库文本转结构体
函数输入： char **str: 数据库文本
         void *retval: 结构体数据
         int flag:  1-STAFF_INFO | 2-STAFF_ATTENDANCE
函数输出： 无
*/
void tb_str2struct(char **str,void *retval,int flag)
{
    Staff sp;
    Attendance ap;
    int col;
    if(flag == 1){
        col = INFO_COL;
        str2num(str[col + 0],NULL,&(sp.id));
        strcpy(sp.code,str[col + 1]);
        strcpy(sp.name,str[col + 2]);
        str2num(str[col + 3],NULL,&(sp.level));
        str2num(str[col + 4],&(sp.salary),NULL);
        str2num(str[col + 5],NULL,&(sp.state));
        memcpy(retval,&sp,sizeof(sp));
        return ;
    } else if(flag == 2) {
        col = ATTEND_COL;
        str2num(str[col + 0],NULL,&(ap.id));
        strcpy(ap.time,str[col + 1]);
        str2num(str[col + 2],NULL,&(ap.clockin));
        memcpy(retval,&ap,sizeof(ap));
        return ;
    }
}

/*打印错误信息
函数名称： tb_err_print
函数功能： 打印错误信息
函数输入： int errno:  错误码
函数输出： 无
*/
void tb_err_print(int errno)
{
    switch (errno)
    {
    case EITEM: printf("对象不存在\n") ;break;
    case ECODE: printf("密码错误\n") ;break;
    case EUPDATE: printf("更新失败\n") ;break;
    case ELOADED: printf("账号已登录\n") ;break;
    case EADD: printf("添加失败\n") ;break;
    case EDEL: printf("删除失败\n") ;break;
    case ENAME: printf("用户名错误或已存在\n") ;break;
    case ECONNECT: printf("通讯错误\n") ;break;
    case ECLOSE: printf("对方关闭\n") ;break;
    case EFIND: printf("查询失败\n") ;break;
    default: printf("错误码不存在\n") ;break;
    }
}

/*查询表格名输入
函数名称： tb_check
函数功能： 查询表格名输入
函数输入： char *tableN:  
函数输出： 1-STAFF_INFO | 2-STAFF_ATTENDANCE | <0-error code
*/
int tb_check(char *tableN)
{
    if(!strcmp(tableN,"STAFF_INFO")){ // STAFF_INFO
        return 1;
    } else if (!strcmp(tableN,"STAFF_ATTENDANCE")) { // STAFF_ATTENDANCE
        return 2;
    } else {        
        return EITEM;
    }
}

/*查询表格存在
函数名称： tb_isexist
函数功能： 查询表格存在
函数输入： sqlite3* db:
         char *tableN:  
函数输出：int ret:1存在/0不存在
*/
int tb_isexist(sqlite3* db,char *tableN)
{
    char sql[SQLLEN]="select * from ";
    strcat(sql,tableN);   
    if(sqlite3_exec(db, sql, NULL, NULL, NULL) !=0)
        return 0;
    return 1; 
}

/*列出当前数据库下已存的表格
函数名称： tb_list
函数功能： 列出当前数据库下已存的表格
函数输入： sqlite3* db:         
函数输出： 0 sucess/ <0-error code
*/
int tb_show(sqlite3* db)
{
    char sql[SQLLEN] = "SELECT * FROM sqlite_master WHERE type='table' ORDER BY name"; 
    
    char **result = NULL;
    int row, col;                                                                
    if(sqlite3_get_table(db, sql, &result, &row, &col, NULL) !=0) {   
        ERR_SQ("sqlite3_get_table");
        return EFIND; 
    }  
    if(row < 1){
        printf("数据库为空\n");
        return EITEM;
    }
    puts("*******表格清单********");
    for (int i = 0; i <= row; i++){
        for (int j = 0; j < col; j++)
            printf("%s\t",result[i*col + j]);
        putchar(10);
    }
    puts("************************");
    sqlite3_free_table(result); 
    return 0;
}

/*检查表格中元素
函数名称：tb_find
函数功能： 检查表格中元素
函数输入：  sqlite3* db:
          char *tableN: 表名
          char *value: 查询内容
          char *colN：列名
          int mode: >=0 精准搜索 / <0 模糊搜索
          int *rows:符合条件的元素数,填NULL无返回
          void *retval: 返回的结构体指针
          char **retarray:返回字符串数组 填NULL无返回 需sqlite3_free_table(result)
函数输出：0 sucess/ <0-error code
*/
int tb_find(sqlite3* db,char *tableN,char *value,char *colN,
                int mode,int *rows,void *retval,char ***retarray)
{
    //"SELECT * FROM sqlite_master WHERE type='table' ORDER BY name"
    char sql[SQLLEN] = ""; 
    int ret = tb_check(tableN);
    if(ret < 0){
        ERRMSG("error table name");
        return ret;
    }

    if(mode >= 0){
        sprintf(sql,"SELECT * FROM %s WHERE %s='%s'",
                    tableN,colN,value);
    }else{
        sprintf(sql,"SELECT * FROM %s WHERE %s LIKE '%%%s%%'",
                    tableN,colN,value);
    }
    char **result = NULL;
    int row, col;    
    if(sqlite3_get_table(db, sql, &result, &row, &col, NULL)) {  
        ERR_SQ("sqlite3_get_table");
        sqlite3_free_table(result);
        return EITEM;
    } 
    if(rows != NULL)
        *rows = row;
    //拷贝数据至结构体
    if(retval != NULL)
        tb_str2struct(result,retval,ret);
    if(retarray != NULL)
        *retarray = result;
    else
        sqlite3_free_table(result);
    return 0;
}

/*检查表格中元素
函数名称： tb_Search
函数功能： 检查表格中元素
函数输入：  sqlite3* db:
          char *tableN: 表名
          char *value: 查询内容
          char *colN：列名
          int *rows:符合条件的元素数,填NULL无返回
          void *retval: 返回的结构体指针
          char **retarray:返回字符串数组 填NULL无返回 需sqlite3_free_table(result)
函数输出：0 sucess/ <0-error code
*/
int tb_Search(sqlite3* db,char *tableN,char *where,
                int *rows,void *retval,char ***retarray)
{
    //"SELECT * FROM sqlite_master WHERE type='table' ORDER BY name"
    char sql[SQLLEN] = ""; 
    int ret = tb_check(tableN);
    if(ret < 0){
        ERRMSG("error table name");
        return ret;
    }

    sprintf(sql,"SELECT * FROM %s WHERE %s",tableN,where);
    char **result = NULL;
    int row, col;                                                                
    if(sqlite3_get_table(db, sql, &result, &row, &col, NULL) !=0) {   
        ERR_SQ("sqlite3_get_table");
        return EITEM;
    } 
    if(row < 1){
        return EITEM;
    }
    if(rows != NULL)
        *rows = row;
    //拷贝数据至结构体
    if(retval != NULL)
        tb_str2struct(result,retval,ret);
    if(retarray != NULL)
        *retarray = result;
    else
        sqlite3_free_table(result);
    return 0;
}


/*表格插入单条数据
函数名称：tb_add
函数功能：插入表格
函数输入： sqlite3* db:
         char *tableN: 表格名
         void *data: 数据结构体指针
函数输出：0-sucess/ <0-error code
*/
int tb_add(sqlite3* db,char *tableN,void *data)
{
    //"insert into tablename values(value1, value2,……);"
    char sql[SQLLEN] = ""; 
    char uptxt[128]="";
    Staff *sp;
    Attendance *ap;
    int ret = tb_check(tableN);
    if(ret < 0){
        ERRMSG("error table name");
        return ret;
    }
    if(ret == 1){   //STAFF_INFO
        sp = (Staff *)data;
        sprintf(uptxt,"(%d,'%s','%s',%d,%.2f,%d)",
                    sp->id,sp->code,sp->name,
                    sp->level,sp->salary,sp->state);
    } else {    //ATTENDANCE
        ap = (Attendance *)data; 
        sprintf(uptxt,"(%d,'%s',%d)",
                    ap->id,ap->time,ap->clockin);        
    }
    sprintf(sql,"insert into %s values %s",tableN,uptxt);
    if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
        ERR_SQ("add");
        return EADD;
    }
    return 0;
}

/*文本批量插入表格
函数名称：tb_mul_add
函数功能：文本批量插入表格
函数输入： sqlite3* db:
         char *tableN: 表格名
         char *path: 文件地址
函数输出：0-sucess/ <0-error code
*/
int tb_mul_add(sqlite3* db,char *tableN,char *path)
{
    char txt[128]="";
    char sql[SQLLEN] = ""; 
    FILE *fp = fopen(path,"r");
    if(fp == NULL){
        return EITEM;
    }
    while (1){
        memset(txt,0,sizeof(txt));
        memset(sql,0,sizeof(sql));
        char *ret = fgets(txt,sizeof(txt),fp);
        if(ret == NULL) {
            fclose(fp);
            break;
        }
        sprintf(sql,"insert into %s values (%s)",tableN,txt);
        if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
            ERR_SQ("add");
            return EADD;
        }
    }
    return 0;
}


/*删除表格单条数据
函数名称：tb_delete
函数功能：删除元素
函数输入： sqlite3 *db
         char *tableN: 
         char *condition：查询条件 columnx = valuex and columnx = valuex
函数输出：0-sucess/ <0-error code
*/
int tb_delete(sqlite3 *db,char *tableN,char *condition)
{
    //delete from table_name where columnx = valuex
    char sql[SQLLEN]="";
    int ret = tb_check(tableN);
    if(ret < 0){
        ERRMSG("error table name");
        return ret;
    }

    sprintf(sql,"delete from %s where %s",tableN,condition);
    if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
        ERR_SQ("delete");
        return EDEL;
    }
    return 0;
}

/*更新单条表格数据
函数名称： tb_modify
函数功能： 更新表格数据
函数输入： sqlite3* db:
         char *tableN: 表格名
         int id:
         void *data: 数据结构体指针
函数输出：0-sucess/ <0-error code
*/
int tb_modify(sqlite3* db,char *tableN,int id,void *data)
{
    //UPDATE table_name SET column1 = value1, .... WHERE columnx = valuex
    char sql[SQLLEN]="";
    char uptxt[128]="";
    Staff *sp;
    Attendance *ap;
    int ret = tb_check(tableN);
    if(ret < 0){
        ERRMSG("error table name");
        return ret;
    }
    if(ret == 1){   //STAFF_INFO
        sp = (Staff *)data;
        sprintf(uptxt,
            "code = '%s',name = '%s',level = %d,salary = %.2f,state = %d",
            sp->code,sp->name,sp->level,sp->salary,sp->state);        
    } else {    //ATTENDANCE
        ap = (Attendance *)data; 
        sprintf(uptxt,"time = '%s',clockin = %d",ap->time,ap->clockin);        
    }

    sprintf(sql,"UPDATE %s SET %s WHERE id = %d",
                tableN,uptxt,id);
    if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
        ERR_SQ("update");
        return EUPDATE;
    }
    return 0;
}


/*更新单条表格数据
函数名称： tb_update
函数功能： 更新表格数据
函数输入： sqlite3* db:
         char *tableN: 表格名
         char *where:
         char *set:
函数输出：0-sucess/ <0-error code
*/
int tb_update(sqlite3* db,char *tableN,char *where,char *set)
{
    //UPDATE table_name SET column1 = value1, .... WHERE columnx = valuex
    char sql[SQLLEN]="";
    int ret = tb_check(tableN);
    if(ret < 0){
        ERRMSG("error table name");
        return ret;
    }
    sprintf(sql,"UPDATE %s SET %s WHERE %s",
                tableN,set,where);
    if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
        ERR_SQ("update");
        return EUPDATE;
    }
    return 0;
}

/*检查id重复
函数名称： tb_idcheck
函数功能： 检查id重复
函数输入： sqlite3 *db:
         int id:
函数输出：0-不重复 | 1-重复 | <0 错误码
*/
int tb_idcheck(sqlite3 *db,int id)
{
    /*
    char value[32]="";
    sprintf(value,"%d",id);
    char **result = tb_find(db,"STAFF_INFO",value,"id",1,NULL);
    if(result == NULL){
        return 0;
    }
    sqlite3_free_table(result);
    return 1;
    */

    char sql[SQLLEN]=""; 
    sprintf(sql,"insert into STAFF_INFO (id) values(%d)",id);
    if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
        return 1;
    }
    memset(sql,0,sizeof(sql));
    sprintf(sql,"delete from STAFF_INFO where id = %d",id);
    if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
        ERR_SQ("delete");
        return EDEL;
    }
    return 0; 
}



