#include "httpresponse.h"
#include "xxsocket.h"

static string bad_message = "HTTP/1.0 400 Bad Message\r\nServer: XXServer\r\n"
        "Content-Length: 15\r\n\r\n"
        "400 bad message";


static string not_found_message = "HTTP/1.0 404 Not Found\r\nServer: XXServer\r\n"
        "Content-Length: 13\r\n\r\n"
        "404 not found";

static string success_head = "HTTP/1.0 200 Ok\r\nServer: XXServer\r\n";

static string websocket_head = "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n\r\n";



HttpResponse::HttpResponse(XXSocket *_socket)
    :m_socket(_socket)
{
}

int HttpResponse::WriteBodyData(const char *buf, int len)
{
    char tmp_buf[1024];
    MBuffer *buffer = m_socket->GetNewBuffer();
    buffer->AddBytes(success_head.data(),success_head.length());

    int n =0;
    for(StringMap::iterator it = m_header_map.begin();
        it != m_header_map.end();++it)
    {
        n = sprintf(tmp_buf,"%s:%s\r\n",it->first.c_str(),it->second.c_str());
        buffer->AddBytes(tmp_buf,n);
    }

     n = sprintf(tmp_buf,"%Content-Length:%d\r\n\r\n",len);
     if(len > 0)
     {
         buffer->AddBytes(tmp_buf,n);
         buffer->AddBytes(buf,len);
     }


     m_socket->StartSend();
     return 0;
}

int HttpResponse::Response404()
{
   return ResponseData(not_found_message.data(),not_found_message.length());
}

int HttpResponse::Response400()
{
   return ResponseData(bad_message.data(),bad_message.length());
}

void HttpResponse::SetHeader(string key, string value)
{
    m_header_map[key] = value;
}

void HttpResponse::Reset()
{
    m_header_map.clear();
}

void HttpResponse::Close()
{
    m_socket->Close();
}

int HttpResponse::ResponseWebsocket(const char * auth)
{
    char res_data[1024];
    int n = sprintf(res_data,websocket_head.c_str(),auth);
    return ResponseData(res_data,n);
}


int HttpResponse::ResponseData(const char *data,int len)
{
    MBuffer *buffer = m_socket->GetNewBuffer();
    buffer->AddBytes(data,len);
    m_socket->StartSend();
    return 0;
}

