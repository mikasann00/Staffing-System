#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "_tcp.h"
#include "cilent.h"

/*登录
函数名称： LoginFun
函数功能： 登录
函数输入： int fds
        Staff *retval:登录对象数据
函数输出：id
*/
void LoginFun(int fds,Staff *retval)
{
    Staff sp;
    struct order rcvbuf;
    int ret;
    while (1){
        printf("输入ID: >>> ");
        INPUTCHECK("%d",(sp.id));
        printf("输入密码: >>> ");
		fgets(sp.code,sizeof(sp.code),stdin);
		sp.code[strlen(sp.code)-1]=0;
        ret = TcpSnd(fds,ALOAD,(char *)&sp,sizeof(sp));
        if(ret){
            ERRMSG("通讯错误,请重试");
            continue;
        }
        ret = TcpRcv(fds,&rcvbuf);
        if(ret){
            ERRMSG("通讯错误,请重试");
            continue;
        }
        
        switch(rcvbuf.type){
        case AACK:
            memcpy(retval,rcvbuf.txt,sizeof(Staff)); //获取登录对象数据
            return ;
        case AERR:
            if(!strcmp(rcvbuf.txt,"对象不存在")){
                puts(rcvbuf.txt);
            }else if(!strcmp(rcvbuf.txt,"error loaded")){ //连续错误3次
                puts(rcvbuf.txt);
                puts("连续输入错误，已退出");
                exit(1);
            }else{ //用户名错误或密码错误,剩余尝试机会%d次
                puts(rcvbuf.txt);
            }
            break;
        default: ERRMSG("error type"); break;
        }
    }// end while       
}

/*注册
函数名称： RegisterFun
函数功能： 注册
函数输入： int fds
函数输出：无
*/
void RegisterFun(int fds)
{
    Staff sp;
    struct order rcvbuf;
    int ret;
    while (1){
        char txt[64]="";
        printf("输入ID (quit 退出): >>> ");
        fgets(txt,sizeof(txt),stdin);
        txt[strlen(txt)-1]=0;

        if(!strcmp(txt,"quit")){
            TcpSnd(fds,AACK,"退出功能",0);
            break;
        }

        if(txt[0]>='0' && txt[0]<='9'){
            sp.id = atoi(txt);
        } else {
            printf("错误输入\n");
            continue;
        }         
		
        ret = TcpSnd(fds,AREG,(char *)&sp,sizeof(sp));
        if(ret){
            ERRMSG("通讯错误,请重试");
            continue;
        }
        ret = TcpRcv(fds,&rcvbuf);
        if(ret){
            ERRMSG("通讯错误,请重试");
            continue;
        }
        if(rcvbuf.type == AERR && !strcmp(rcvbuf.txt,"id重复")){
            puts(rcvbuf.txt);
            continue;
        }
         
        printf("输入密码: >>> ");
		fgets(sp.code,sizeof(sp.code),stdin);
		sp.code[strlen(sp.code)-1]=0;
        printf("输入姓名： >>> ");
        fgets(sp.name,sizeof(sp.name),stdin);
		sp.name[strlen(sp.name)-1]=0;
        printf("输入等级： >>> ");
        INPUTCHECK("%d",(sp.level));
        printf("输入薪水： >>> ");
        INPUTCHECK("%f",(sp.salary)); 
        sp.state = 1;

        ret = TcpSnd(fds,AREG,(char *)&sp,sizeof(sp));
        if(ret){
            ERRMSG("通讯错误,请重试");
            continue;
        }
        ret = TcpRcv(fds,&rcvbuf);
        if(ret){
            ERRMSG("通讯错误,请重试");
            continue;
        }
        puts(rcvbuf.txt);        
    }//end while
}


/*查询用户信息
函数名称： SearchUserFun
函数功能： 查询用户信息
函数输入： 
函数输出：choose
*/
void SearchUserFun(int fds,Staff *curuser)
{

    Staff sp;
    struct order rcvbuf;
    char txt[128]="";
    while (1){      
        if(curuser->level > 5){  //管理员用户
            printf("输入ID 或 姓名 (quit 退出): >>> ");	
            fgets(txt,sizeof(txt),stdin);
            txt[strlen(txt)-1]=0;

            if(!strcmp(txt,"quit"))
                break;

            if(txt[0]>='0' && txt[0]<='9'){
                sp.id = atoi(txt);
                memset(txt,0,sizeof(txt));
                sprintf(txt,"id = %d",sp.id);
            } else {
                strcpy(sp.name,txt);
                memset(txt,0,sizeof(txt));
                sprintf(txt,"name = '%s'",sp.name);
            } 
        }else{  //普通用户
            sprintf(txt,"id = %d",curuser->id);
        }          
        TcpSnd(fds,AFINDUSER,txt,0);
        TcpRcv(fds,&rcvbuf);
        if(rcvbuf.type == AERR){
            puts(rcvbuf.txt);
            continue;
        }
        puts(rcvbuf.txt);
        if(curuser->level < 5){
            break;
        }
    }//end while
}

/*查询打卡记录
函数名称： SearchAttendFun
函数功能： 查询打卡记录
函数输入： int fds
函数输出：无
*/
void SearchAttendFun(int fds,Staff *curuser)
{
    Staff sp;
    struct order rcvbuf;
    char txt[128]="";
    while (1){     
        if(curuser->level > 5){  //管理员用户   
            printf("输入ID (quit 退出): >>> ");	
            fgets(txt,sizeof(txt),stdin);
            txt[strlen(txt)-1]=0;

            if(!strcmp(txt,"quit"))
                break;

            if(txt[0]>='0' && txt[0]<='9'){
                sp.id = atoi(txt);
                memset(txt,0,sizeof(txt));
                sprintf(txt,"id = %d",sp.id);
            } else {
                printf("错误输入\n");
                continue;
            } 
        }else{  //普通用户
            sprintf(txt,"id = %d",curuser->id);
        } 

        TcpSnd(fds,AFINDATTD,txt,0);
        TcpRcv(fds,&rcvbuf);
        if(rcvbuf.type == AERR){
            puts(rcvbuf.txt);
            continue;
        }
        puts(rcvbuf.txt);
        if(curuser->level < 5){
            break;
        }
    }//end while
}


/*修改用户信息
函数名称： ChangeFun
函数功能： 修改用户信息
函数输入： int fds
函数输出：无
*/
void ChangeFun(int fds,Staff *curuser)
{
    Staff sp;
    struct order rcvbuf;
    char txt[128]="";
    while (1){  
        memset(txt,0,sizeof(txt));
        if(curuser->level > 5){  //管理员用户   
            printf("输入ID (quit 退出): >>> ");	
            fgets(txt,sizeof(txt),stdin);
            txt[strlen(txt)-1]=0;
            if(!strcmp(txt,"quit"))
                break;  

            if(txt[0]>='0' && txt[0]<='9'){
                sp.id = atoi(txt);
                memset(txt,0,sizeof(txt));
                sprintf(txt,"%d",sp.id);
            } else {
                printf("错误输入\n");
                continue;
            }   
        }else{  //普通用户
            sprintf(txt,"%d",curuser->id);
        }
        //发送id
        TcpSnd(fds,ACHANGEUSER,txt,0);

        TcpRcv(fds,&rcvbuf);
        if(rcvbuf.type == AERR){ //无匹配项
            puts(rcvbuf.txt);
            continue;
        }
        puts(rcvbuf.txt);
        //选择修改项
        memset(txt,0,sizeof(txt));
        int choose;
        char tmptxt[64]="";
        while(1){
            memset(tmptxt,0,sizeof(tmptxt));
            choose = modifymenu(curuser->level);
            switch(choose){
            case 1: //code
                printf("新密码 >>> ");
                fgets(sp.code,sizeof(sp.code),stdin);
                sp.code[strlen(sp.code)-1]=0;
                sprintf(tmptxt,", code='%s' ",sp.code);
                break;
            case 2: //name
                printf("新姓名 >>> ");
                fgets(sp.name,sizeof(sp.name),stdin);
                sp.name[strlen(sp.name)-1]=0;
                sprintf(tmptxt,", name='%s' ",sp.name);
                break;
            case 3: 
                if(curuser->level > 5)
                    sprintf(tmptxt," %c id=%d ",'\n',sp.id);
                else
                    sprintf(tmptxt," %c id=%d ",'\n',curuser->id);
                break;
            case 4: //level
                printf("新等级>>> ");
                INPUTCHECK("%d",sp.level);
                sprintf(tmptxt,", level=%d ",sp.level);
                break;
            case 5: //salary  
                printf("新薪水 >>> ");              
                INPUTCHECK("%f",sp.salary);
                sprintf(tmptxt,", salary=%.2f ",sp.salary);
                break;
            }
            tmptxt[0]=' ';
            strcat(txt,tmptxt);
            if(choose == 3)
                break;
        }
        TcpSnd(fds,ACHANGEUSER,txt,0);
        TcpRcv(fds,&rcvbuf);
        puts(rcvbuf.txt);
        if(curuser->level <= 5)
            return;
    }//end while
}


/*删除用户
函数名称： DeleteFun
函数功能： 删除用户
函数输入： int fds
函数输出：无
*/
void DeleteFun(int fds,Staff *curuser)
{
    if(curuser->level <= 5)
        return;
    struct order rcvbuf;
    char txt[128]="";
    char tmptxt[64]="";
    while (1){  
        memset(txt,0,sizeof(txt));
        memset(tmptxt,0,sizeof(tmptxt));
        printf("输入ID (quit 退出): >>> ");	
        fgets(tmptxt,sizeof(tmptxt),stdin);
        tmptxt[strlen(tmptxt)-1]=0;
        if(!strcmp(tmptxt,"quit"))
            break;    
        sprintf(txt," id=%s ",tmptxt);

        TcpSnd(fds,ADELUSER,txt,0);
        TcpRcv(fds,&rcvbuf);
        puts(rcvbuf.txt);
    }  //end while          
}

/*打卡
函数名称： Clockinfun
函数功能： 删除用户
函数输入： int fds
函数输出：无
*/
void Clockinfun(int fds,Staff *curuser)
{
    struct order rcvbuf;
    Attendance ap;
    ap.id = curuser->id;
    
    TcpSnd(fds,ACLOCKIN,(char*)&ap,sizeof(ap));
    TcpRcv(fds,&rcvbuf);
    puts(rcvbuf.txt);
}


int menu(void)
{
    puts("*********************");
    puts("       1.登录");
    puts("       2.退出");
    puts("*********************");
    int choose = 0;
    do{
        INPUTCHECK("%d",choose);
        if(choose >= 1 && choose <= 2)
            break;
        printf("选项不存在\n");
    }while(1);
    return choose;
}

int adminmenu(void)
{
    puts("*********************");
    puts("       1.注册新用户");
    puts("       2.查询用户信息");
    puts("       3.查询打卡记录");
    puts("       4.修改用户信息");
    puts("       5.删除用户");
    puts("       6.返回上一级");
    puts("*********************");
    int choose = 0;
    do{
        INPUTCHECK("%d",choose);
        if(choose >= 1 && choose <= 6)
            break;
        printf("选项不存在\n");
    }while(1);
    return choose;
}

int admincontrol(int fds,Staff *sp)
{
    while(1){
        int choose = adminmenu();
        switch(choose){
        case 1:    //1.注册新用户"
            RegisterFun(fds);
            break;
        case 2:    //2.查询用户信息"
            SearchUserFun(fds,sp);
            break;
        case 3:    //3.查询打卡记录"
            SearchAttendFun(fds,sp);
            break;
        case 4:    //4.修改用户信息"
            ChangeFun(fds,sp);
            break;
        case 5:    //5.删除用户"
            DeleteFun(fds,sp);
            break;
        case 6:    //6.返回上一级"
            return 6;
            break;
        }  
    }  
}

int usercontrol(int fds,Staff *sp)
{
    while(1){
        int choose = usermenu();
        switch(choose){
        case 1:    //1.查询用户信息"
            SearchUserFun(fds,sp);
            break;
        case 2:    //2.查询打卡记录"
            SearchAttendFun(fds,sp);
            break;
        case 3:    //3.修改用户信息"
            ChangeFun(fds,sp);
            break;
        case 4:    //4.打卡"
            Clockinfun(fds,sp);
            break;
        case 5:    //5.返回上一级"
            return 5;
            break;
        }  
    }
}

int usermenu(void)
{
    puts("*********************");
    puts("       1.查询用户信息");
    puts("       2.查询打卡记录");
    puts("       3.修改用户信息");
    puts("       4.打卡");
    puts("       5.返回上一级");
    puts("*********************");
    int choose = 0;
    do{
        INPUTCHECK("%d",choose);
        if(choose >= 1 && choose <= 5)
            break;
        printf("选项不存在\n");
    }while(1);
    return choose;
}

int modifymenu(int level)
{
    puts("*********************");
    puts("       1.修改密码");
    puts("       2.修改用户名");
    puts("       3.返回上一级");
    if(level > 5){
    puts("       4.修改等级");
    puts("       5.修改工资");
    }
    puts("*********************");
    int choose = 0;
    do{
        INPUTCHECK("%d",choose);
        if(level > 5 && (choose >= 1 && choose <= 5))
            break;
        if(level <= 5 && (choose >= 1 && choose <= 3))
            break;        
        printf("选项不存在\n");
    }while(1);
    return choose;
}