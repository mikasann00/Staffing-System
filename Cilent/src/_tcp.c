#include "_tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>    
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>



/***********define****************/

/*
函数名称：
函数功能： 
函数输入： 
函数输出：无
*/

/************** func ******************/
/*TCP发送
函数名称： TcpSnd
函数功能： TCP发送
函数输入： int fds：
         char type：
         char *txt：
函数输出：0-sucess/ -1-false
*/
int TcpSnd(int fds,char type,char *txt,unsigned int size)
{
    struct order sndbuf ;
    bzero((char *)&sndbuf,sizeof(sndbuf));
    sndbuf.type = type; 
    if(!size)
       size = strlen(txt);
    sndbuf.num = htonl(size);
    memcpy(sndbuf.txt,txt,size);
    ssize_t sndret = send(fds,(char *)&sndbuf,sizeof(sndbuf),0);
    if(sndret < 0){
        ERRMSG("send require");
        return -1;
    } 
    return 0;
}

/*TCP接收
函数名称： TcpRcv
函数功能： TCP接收
函数输入： int fds：
         struct order *buf：存储接收信息的结构体
函数输出：0-sucess/ -1-对方关闭 /-2-error
*/
int TcpRcv(int fds,struct order *buf)
{
    struct order sndbuf ;
    bzero((char *)&sndbuf,sizeof(sndbuf));
    ssize_t rcvret = recv(fds, (void *)&sndbuf, sizeof(sndbuf), 0);
    if(rcvret == 0){
        fprintf(stderr,"[%d] 对方关闭\n",fds);
        //close(fds);
        return -1;
    }else if(rcvret < 0){
        ERRMSG("recv");
        return -2;
    }
    memcpy(buf,&sndbuf,sizeof(sndbuf));
    buf->num = ntohl(sndbuf.num);
    return 0;
}


/*TCP初始化
函数名称： TcpInit
函数功能： TCP初始化
函数输入： int *fd:返回的监听套接字
          int lisnum:监听数量上限
函数输出：无
*/
void TcpInit(int *fd,int lisnum)
{
    //创建监听套接字
    int fdmon = socket(AF_INET,SOCK_STREAM, 0);
    if(fdmon < 0){
        ERRMSG("socket");
        exit(1);
    }
    //允许本地端口快速重用
    int reuse = 1;
    if(setsockopt(fdmon, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))<0)
    {   
        ERRMSG("setsockopt");
        return ; 
    }
    //绑定服务器ip和端口
    struct sockaddr_in sin;
    sin.sin_family      = AF_INET;
    sin.sin_port        = htons(PORT);
    sin.sin_addr.s_addr = inet_addr(IP);
    if(bind(fdmon, (struct sockaddr*)&sin, sizeof(sin)) < 0){
        ERRMSG("bind");
         exit(1);
    }
    printf("Ip、端口绑定 done!\n");
    //监听套接字设为被动监听
    if(listen(fdmon,lisnum) < 0){
        ERRMSG("listen");
        exit(1);
    }
    printf("设置监听 done！\n");
    *fd = fdmon;
}




