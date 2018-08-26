#include "xxWS/xxserver.h"

const char *success_head = "HTTP/1.1 200 Ok\r\nServer: XXServer\r\n"
       //"Connection: close\r\n"
        "Content-Length: %d\r\n"
        "Content-Type: text/html\r\n\r\n";
void HandleHttp(HttpRequest *req,HttpResponse *res)
{
        const char *method = req->Method();
        if(strcmp(method,"GET") != 0)
        {
            res->Response400();
            return;
        }
        const char *path = req->Path();
        if(strcmp(path,"/") == 0) path = "/index.html";
        char total_path[1024];
        sprintf(total_path,"./html%s",path);

        FILE *fp = fopen(total_path,"rb");
        if(fp == NULL){
            res->Response404();
            return;
        }

        fseek(fp,0,SEEK_END);
        int file_len = ftell(fp);
        rewind(fp);

        char buff[4096] = {0};
        int n = sprintf(buff,success_head,file_len);
        res->ResponseData(buff,n);

        while(file_len > 0)
        {
                memset(buff,0,sizeof(buff));
                n = fread(buff,1,4096,fp);
                if(n <= 0) {
                    printf("close %d,%d\n",n,file_len);
                    break;
                }
                res->ResponseData(buff,n);
                file_len -= n;
        }

        fclose(fp);
}



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

    xx_server.SetHttpHandleFun(HandleHttp);
    xx_server.SetHttpCloseFun([](HttpRequest *req,HttpResponse *res)
    {
        printf("http close --------\n");
    });

    return xx_server.Listen(8080);
}


