#ifndef NET_COMMON_H
#define NET_COMMON_H

#include <stdio.h>




#include <stdio.h>
#include <errno.h>
#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#define IS_SOCK_ERR_BLOCK (WSAGetLastError() == WSAEWOULDBLOCK)
#else
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#define IS_SOCK_ERR_BLOCK (errno == EAGAIN || errno == EWOULDBLOCK)
#endif



#define CHECK_ERROR_RETURN(x,str) \
{ \
    int ret = x ^ 0xffffffff; \
    if(ret == 0) \
    { \
    printf("%s , %d : ",__FILE__,__LINE__);\
    perror(str); \
    return -1; \
    } \
}

#define SERVER_BACKLOG 128

#ifdef WIN32
#define XXSERVER_LOG_INFO(fmt,...) \
     printf("%s , %d : ",__FILE__,__LINE__);\
     printf(fmt,##__VA_ARGS__);

#define XXSERVER_LOG_ERROR(fmt,...) XXSERVER_LOG_INFO(fmt,##__VA_ARGS__)

#define INIT_XX_SERVER \
WSADATA wsaData;  \
CHECK_ERROR_RETURN(WSAStartup(MAKEWORD(2, 2), &wsaData),"WSAStartUp Error")


#define SET_SOCKET_UNLOCK(fd)\
unsigned long ul=1;\
ioctlsocket(fd,FIONBIO,&ul);  

#define close closesocket 

#define strtok_r strtok_s

#else
#define XXSERVER_LOG_INFO(fmt,...) \
     printf("%s , %d : ",__FILE__,__LINE__);\
     printf(fmt,##__VA_ARGS__);

#define XXSERVER_LOG_ERROR(fmt,...) XXSERVER_LOG_INFO(fmt,##__VA_ARGS__)

#define INIT_SERVER \
 signal(SIGPIPE,SIG_IGN);

#define SET_SOCKET_UNLOCK(fd)\
int flags = fcntl(fd, F_GETFL, 0);            /*获取文件的flags值。*/ \
fcntl(fd, F_SETFL, flags | O_NONBLOCK);   /*设置成非阻塞模式 */
#endif

#define RETURN_ERROR -1
#define RETURN_NO 0
#define RETURN_PARSE_FINISHED 1
#define RETURN_CLIENT_CLOSE 2


#endif // COMMON_TOOL_H
