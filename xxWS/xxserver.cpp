#include "xxserver.h"
#include "signal.h"
#include "xxsocket.h"

static bool is_init = false;

XXServer::XXServer()
    :Event(&m_event_manager),
      m_http_handle_fun(NULL),
      m_http_error_fun(NULL),
      m_websocket_msg_fun(NULL),
      m_websocket_close_fun(NULL)
{

}

int XXServer::HandleRead()
{
    int connfd = 0;
    while(1)
    {
        connfd = accept(m_fd,NULL,NULL);
        if(connfd < 0)
        {
            if(!IS_SOCK_ERR_BLOCK) //判断是否因为非阻塞
                Close();
            break;
        }
        SET_SOCKET_UNLOCK(connfd) //设置非阻塞
        XXSocket *xx_socket = GetNewSocket();
        xx_socket->SetFd(connfd);
        m_event_manager.AddEvent(xx_socket,EV_READ);
    }
    return 0;
}

int XXServer::HandleError()
{
    Close();
    return 0;
}

int XXServer::Listen(unsigned short port)
{
	if (!is_init)
	{
		INIT_XX_SERVER //初始化服务器
	}

    m_fd = socket(AF_INET,SOCK_STREAM,0);
    CHECK_ERROR_RETURN(m_fd,"socket");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

	SET_SOCKET_UNLOCK(m_fd) //设置非阻塞

    CHECK_ERROR_RETURN(bind(m_fd,(struct sockaddr *)&addr,sizeof(addr)),"bind");
    CHECK_ERROR_RETURN(listen(m_fd,SERVER_BACKLOG),"listen");

    CHECK_ERROR_RETURN(m_event_manager.Open(),"event");
    m_event_manager.AddEvent(this,EV_READ);

    return m_event_manager.StartLoop();
}

void XXServer::Close()
{
    m_event_manager.RemoveEvent(this);
}

void XXServer::SetHttpHandleFun(HttpHandleFun fun)
{
    m_http_handle_fun = fun;
}

void XXServer::SetHttpCloseFun(HttpCloseFun fun)
{
    m_http_error_fun = fun;
}

void XXServer::PushSocket(XXSocket *_socket)
{
    m_socket_pool.push_back(_socket);
}

XXSocket *XXServer::GetNewSocket()
{
    XXSocket *_socket = NULL;
    if(m_socket_pool.empty())
    {
        _socket = new XXSocket(this);
    }else
    {
        _socket = m_socket_pool.back();
        m_socket_pool.pop_back();
    }
    return _socket;
}
