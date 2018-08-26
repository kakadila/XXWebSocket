#include "xxsocket.h"
#include "xxserver.h"
#include <mbedtls/base64.h>
#include <mbedtls/sha1.h>

XXSocket::XXSocket(XXServer *serv)
    :Event(serv->GetEventManager()),
      m_http_response(this),
      m_web_socket(this)
{
	m_server = serv;
    Reset();
}

int XXSocket::HandleRead()
{
    char buf[1024];
    int ret;
    while(1)
    {
        ret = recv(m_fd,buf,1024,0);
        if(ret < 0 )
        {
            if(!IS_SOCK_ERR_BLOCK) //判断是否因为非阻塞
            {
                Close();
                return RETURN_ERROR;
            }
            break;
        }
        //client close
        else if(ret == 0)
        {
            Close();
            return RETURN_NO;
        }

        if(!m_is_upgrade)
        {
            buf[ret] = 0;
            printf("%s",buf);

            // http request
            if(HandleHttp(buf,ret) == RETURN_ERROR)
            {
                Close();
                return RETURN_ERROR;
            }
        }else
        {
            //web socket...
            //LOG_INFO("handle web socket\n");
            if(HandleWebSocketMessage(buf,ret) == RETURN_ERROR)
            {
                Close();
                return RETURN_ERROR;
            }

        }
    }
    return RETURN_NO;
}

int XXSocket::HandleWrite()
{
    int ret;
    while(!m_write_list.empty())
    {
        MBuffer *buffer = m_write_list.front();
        //将这个buffer的剩余没有写出去的字节写出去
        ret = send(m_fd,buffer->GetRemainBytes(),buffer->GetRemainLen(),0);
        if(ret < 0 )
        {
            if(IS_SOCK_ERR_BLOCK) //判断是否因为非阻塞
                return 0;

            Close();
            return -1;
        }
    //    XXSERVER_LOG_INFO("send len = %d\n",ret);
        //如果这个buffer还有字节没有写出去，则退出等待下一次写事件触发
        if(ret < buffer->GetRemainLen())
        {
            buffer->GetBytes(ret);
            return 0;
        }
        m_write_list.pop_front();
        m_tmp_list.push_back(buffer);
    }

    m_event_mgr->ModEvent(this,EV_READ);
    return 0;
}

int XXSocket::HandleError()
{
    XXSERVER_LOG_INFO("error");
    Close();
    return 0;
}

MBuffer *XXSocket::GetNewBuffer()
{
    MBuffer *buffer;
    if(!m_tmp_list.empty())
    {
        buffer = m_tmp_list.front();
        m_tmp_list.pop_front();
    }else
        buffer = new MBuffer();

    buffer->Clear();
    m_write_list.push_back(buffer);
    return buffer;
}

void XXSocket::Reset()
{
    while(!m_write_list.empty())
    {
        MBuffer *buffer = m_write_list.front();
        m_write_list.pop_front();
        m_tmp_list.push_back(buffer);
    }
    m_user_data = NULL;
    m_is_upgrade = false;
    m_http_request.ReSet();
    m_http_response.Reset();
    m_web_socket.Reset();
}

void XXSocket::Close()
{
   // LOG_INFO("close\n");

    if(m_fd < 0) //had been closed
        return;


    if(m_http_request.IsParseFinished())
    {
        //关闭回调
        HttpCloseFun fun = m_server->GetHttpCloseFun();
        if(fun != NULL && !m_is_upgrade)
            fun(&m_http_request,&m_http_response);

        WebSocketCloseFun web_fun = m_server->GetWebSocketCloseFun();
        if(web_fun != NULL && m_is_upgrade)
            web_fun(&m_web_socket);
    }

    Reset();
    m_event_mgr->RemoveEvent(this); //remove 的时候回关闭
    m_server->PushSocket(this);
}

void XXSocket::StartSend()
{
    if(m_event_mgr->ModEvent(this,EV_WRITE)<0)
    {
        Close();
        XXSERVER_LOG_INFO("mod event error\n");
    }
}


int XXSocket::HandleHttp(char *buf, int len)
{
    if(m_http_request.IsParseFinished())
    {
        m_http_request.ReSet();
        m_http_response.Reset();
    }
    int ret = m_http_request.HandleData(buf,len);
    if(ret<0)
        return RETURN_ERROR;
    else if(ret == RETURN_PARSE_FINISHED)
    {
        char *upgrade = m_http_request.Header("Upgrade");
      //  printf("upgrade %s\n",upgrade);
        if(upgrade != NULL && strcmp(upgrade,"websocket") == 0)
        {
            //websocket
            char *version = m_http_request.Header("Sec-WebSocket-Version");
            if(version == NULL || atoi(version) < 13)
            {
                XXSERVER_LOG_INFO("don't support the version %s\n",version);
                return RETURN_ERROR;
            }

            char *key = m_http_request.Header("Sec-WebSocket-Key");
            if(key == NULL)
            {
                XXSERVER_LOG_INFO("Sec-WebSocket-Key NULL!\n",version);
                return RETURN_ERROR;
            }
            char re_buf[1024];
            int n = GetKey(key,re_buf,1024);
            m_http_response.ResponseWebsocket(re_buf);
            m_is_upgrade = true;  //flag

        }else
        {
            //http
            HttpHandleFun fun = m_server->GetHttpHandleFun();
            if(fun != NULL)
            {
                fun(&m_http_request,&m_http_response);
            }
        }
    }
    return RETURN_NO;
}

int XXSocket::HandleWebSocketMessage(char *buf, int len)
{
    if(m_web_socket.GetIsFinished())
    {
        m_web_socket.Reset();
    }
    int ret = m_web_socket.HandleData(buf,len);
    if(ret == RETURN_ERROR) return ret;
    else if(ret == RETURN_CLIENT_CLOSE)
    {
        m_web_socket.Close("close",5);

    }else if(ret == RETURN_PARSE_FINISHED)
    {
        WebSocketMessageFun fun = m_server->GetWebSocketMsgFun();
        if(fun != NULL)
        {
            fun(&m_web_socket,m_web_socket.GetData(),m_web_socket.GetLen(),m_web_socket.GetOpCode());
        }

    }
    return RETURN_NO;
}

int XXSocket::GetKey(char *key, char *out,int max_size)
{
    char buf[1024];
    unsigned char sha_buf[20];
    size_t n = sprintf(buf,"%s%s",key,"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    mbedtls_sha1( (const unsigned char *)buf,n,sha_buf );

    if(mbedtls_base64_encode( (unsigned char *)out, max_size, &n,sha_buf, 20 ) != 0) return -1;
    return n;
}
