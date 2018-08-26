#ifndef XXSERVER_H
#define XXSERVER_H

#include "eventmanager.h"
#include "net_common.h"
#include <vector>
#include "xxsocket.h"

typedef void (*HttpHandleFun)(HttpRequest *req,HttpResponse *res);
typedef HttpHandleFun HttpCloseFun;


typedef void (*WebSocketMessageFun)(WebSocket *web_socket,char *data,int len,unsigned char op_code);
typedef void (*WebSocketCloseFun)(WebSocket *web_socket);


class XXServer:public Event
{
    typedef std::vector<XXSocket *> SocketPool;
    friend class XXSocket;
public:
    XXServer();
    virtual ~XXServer(){}
    virtual int HandleRead();
    virtual int HandleError();

    int Listen(unsigned short port);
    void Close();

    void SetHttpHandleFun(HttpHandleFun fun);
    void SetHttpCloseFun(HttpCloseFun fun);

    void SetWebSocketMsgFun(WebSocketMessageFun fun) {m_websocket_msg_fun = fun;}
    void SetWebSocketCloseFun(WebSocketCloseFun fun){m_websocket_close_fun = fun;}
private:
    EventManager *GetEventManager(){return &m_event_manager;}
    void PushSocket(XXSocket *_socket);
    XXSocket *GetNewSocket();

    HttpHandleFun GetHttpHandleFun(){return m_http_handle_fun;}
    HttpCloseFun GetHttpCloseFun(){return m_http_error_fun;}

    WebSocketMessageFun GetWebSocketMsgFun(){return m_websocket_msg_fun;}
    WebSocketCloseFun GetWebSocketCloseFun(){return m_websocket_close_fun;}
private:
    EventManager m_event_manager;
    SocketPool m_socket_pool;

    HttpHandleFun m_http_handle_fun;
    HttpCloseFun m_http_error_fun;

    WebSocketMessageFun m_websocket_msg_fun;
    WebSocketCloseFun m_websocket_close_fun;
};

#endif // XXSERVER_H
