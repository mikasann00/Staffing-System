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

int main(int argc, char *argv[])
{
    
    if (argc < 2){
        puts("请输入服务器IP");
        exit(1);
    }

    //创建套接字
    int fds = socket(AF_INET, SOCK_STREAM, 0);
    if (fds < 0)
        ERRMSG("socket");
    //连接服务器
    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(fds, (struct sockaddr *)&my_addr, sizeof(my_addr)))
        ERRMSG("connect");
    printf("连接服务器 [%s:%d] done!\n", inet_ntoa(my_addr.sin_addr),
           ntohs(my_addr.sin_port));
    //主循环
    Staff sp;
    int ret;
    while (1)
    {
        int choose = menu();
        switch (choose){
        case 1: //load
            LoginFun(fds,&sp);
            if(sp.level>5){
                ret = admincontrol(fds,&sp);
                if(ret == 6)
                    TcpSnd(fds,AEXIT,"exit",0);
            }else{
                ret = usercontrol(fds,&sp);
                if(ret == 5)
                    TcpSnd(fds,AEXIT,"exit",0);
            }
            break;
        case 2: //exit
            goto EXITFLAG;
            break;        
        }
        
        printf("输入任意字符清屏>>>");
        while (getchar() != 10);
        system("clear");
    }
    /*=============================================*/
EXITFLAG:
    close(fds);
    return 0;
}