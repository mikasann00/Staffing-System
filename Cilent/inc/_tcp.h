#ifndef __TCP_H__
#define __TCP_H__

#include "cilent.h"



/*************func****************/
void TcpInit(int *fd,int lisnum);

int TcpSnd(int fds,char type,char *txt,unsigned int size);

int TcpRcv(int fds,struct order *buf);
/*********************************/


#endif