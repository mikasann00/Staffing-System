#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>    
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sqlite3.h>
#include "_tcp.h"
#include "_sqlite3.h"

typedef struct
{
    sqlite3 *db;
    int fds;
}PTHREAD_ST; //线程传参


void *thread_fun(void *arg)
{
	pthread_detach(pthread_self());
    PTHREAD_ST thval = *(PTHREAD_ST*)arg;
    int fds = thval.fds;
    sqlite3 *db = thval.db;
    int ret,exitflag = 0;
    Staff sp;   //当前用户
    while(1){
        //登录检查
        ret = order2struct(&sp,fds);
        if(ret){
            tb_err_print(ret);
            if(ret == ECLOSE)
                break;
            continue;
        }    
        ret = Login_fun(db,&sp,fds);
        if(ret){
            tb_err_print(ret);
            if(ret == ECLOSE)
                break;
            continue;
        } 
        
        struct order sndbuf;
        char txt[TXTLEN];
        Staff tmp;
        while(1){
            bzero((char *)&sndbuf,sizeof(sndbuf));
            bzero(txt,sizeof(txt));
            //接受信息
            ret = TcpRcv(fds,&sndbuf);
            if(ret == -1) break;
            //检查返回值
            switch(sndbuf.type){
            case AREG: //注册
                ret = Register_fun(db,&sndbuf,fds);
                break;
            case AFINDUSER: //查询user信息
            case AFINDATTD: //查询打卡信息
                ret = Search_fun(db,&sndbuf,fds);
                break;
            case ACHANGEUSER: //修改user信息
                ret = Change_fun(db,&sndbuf,fds);
                break;
            case ADELUSER: //删除用户
                ret = Delete_fun(db,&sndbuf,fds);
                break;
            case ACLOCKIN:  //打卡
                ret = Clockin_fun(db,&sndbuf,fds);
                break;
            case AEXIT:
                exitflag = 1;
                break;
            default: 
                ret = ECONNECT;
                break;
            }
            if(exitflag)
                break;
            if(ret){
                tb_err_print(ret);
                continue;
            }            
        
        } // end while
        //更新用户状态
        char idstr[64]="";
        sprintf(idstr,"%d",sp.id);
        ret = tb_find(db,"STAFF_INFO", idstr,"id",1,NULL,&sp,NULL);
        sp.state = 1;
        tb_modify(db,"STAFF_INFO",sp.id,&sp);
        puts("更新用户状态-下线");
        
    }
    close(fds);
    puts("客户端关闭");
    return NULL;
}





    
int main(int argc,char *argv[])
{
    //初始化
    sqlite3 *db;
    if(sqlite3_open("./my.db", &db) != 0){
        ERRMSG("sqlite3_open");
        return -1;
    }
    int ret = Initial_fun(db);
    if(ret){
        tb_err_print(ret);
        return -1;
    }
    int fdmon = -1;
    TcpInit(&fdmon,64);
    if(fdmon < 0){
        ERRMSG("TcpInit");
        return -1;
    }


    //网络监听
    struct sockaddr_in cin;
    socklen_t addrlen = sizeof(cin);
    PTHREAD_ST thval;    //线程传参
    while (1)
    {
        //获取链接客户端的套接字
        int fds = accept(fdmon,(struct sockaddr *)&cin,&addrlen);
        if(fds < 0){
            ERRMSG("accept");
            continue;
        }
        printf("连接客户端[%s:%d] done!\n",inet_ntoa(cin.sin_addr),ntohs(cin.sin_port));
        thval.fds = fds;
        thval.db = db;
        //创建线程
        pthread_t snd;
        if(pthread_create(&snd,NULL,thread_fun,&thval) < 0){
            ERRMSG("pthread_create snd");
            continue;
        }
    }
    close(fdmon);
    return 0;
}
