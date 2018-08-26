# XXWebSocket
A simple websocket server  
You can open it with qt, or compile it with makefile. I've provided a makefile for the Windows platform, and compile it under Linux with - lpthread.  
Main.cpp is a test file that can be used for reference.  
```C++
#include "xxWS/xxserver.h"
#include <string.h>

const char *success_head = "HTTP/1.1 200 Ok\r\nServer: XXServer\r\n"
       //"Connection: close\r\n"
        "Content-Length: 5\r\n"
        "Content-Type: text/html\r\n\r\n"
        "Hello";


int main()
{
    XXServer xx_server;

    xx_server.SetWebSocketMsgFun([](WebSocket *web_socket,char *data,int len,unsigned char op_code){
        printf("%s\n",data);
        web_socket->Close("Close",op_code);
    });
    xx_server.SetWebSocketCloseFun([](WebSocket *web_socket){
        printf("websocket close --------\n");
    });

    xx_server.SetHttpHandleFun([](HttpRequest *req,HttpResponse *res){
        res->ResponseData(success_head,strlen(success_head));
    });
    xx_server.SetHttpCloseFun([](HttpRequest *req,HttpResponse *res)
    {
        printf("http close --------\n");
    });

    return xx_server.Listen(8080);
}

```
