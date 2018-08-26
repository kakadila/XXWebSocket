#ifndef XXSOCKET_H
#define XXSOCKET_H

#include "net_common.h"
#include "eventmanager.h"
#include "mbuffer.h"
#include <list>
#include "httprequest.h"
#include "httpresponse.h"
#include "websocket.h"

class XXServer;
class XXSocket:public Event
{
    typedef std::list<MBuffer *> MBufferList;
public:
    XXSocket(XXServer *serv);

    virtual int HandleRead();
    virtual int HandleWrite();
    virtual int HandleError();

    MBuffer *GetNewBuffer();
    void Reset();

    void Close();
    void StartSend();

    void SetUserData(void *data){m_user_data = data;}
    void *GetUserData(){return m_user_data;}
private:
    int HandleHttp(char *buf,int len);
    int HandleWebSocketMessage(char *buf,int len);
    int GetKey(char *key, char *out, int max_size);
private:

    MBufferList m_write_list; //写缓冲列表
    MBufferList m_tmp_list; //列表缓冲池
    XXServer *m_server;
    HttpRequest m_http_request; //http request handle
    HttpResponse m_http_response;
    WebSocket m_web_socket;

    bool m_is_upgrade; //is websocket ?
    void *m_user_data;
};

#endif // XXSOCKET_H
