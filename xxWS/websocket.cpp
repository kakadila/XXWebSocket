#include "websocket.h"
#include "xxsocket.h"

WebSocket::WebSocket(XXSocket *_socket)
    :m_socket(_socket)
{
    Reset();
}

int WebSocket::HandleData(const char *data, int len)
{
    m_frame_buffer.AddBytes(data,len);

    while(m_frame_buffer.GetRemainLen() > 0)
    {
        if(m_parse_status == ParseData)
        {
            if(m_frame_buffer.GetRemainLen()  >= m_payload_len)
            {
                HandlePayloadData();
                if(m_is_finished)
                {
                   // *(m_data_buffer.GetBuffer()+m_data_buffer.GetLen()) = 0; //string end
                    m_parse_status = ParseFinised;
                    return RETURN_PARSE_FINISHED;
                }
            }
            return RETURN_NO;
        }

        if(m_parse_status == ParseHead && m_frame_buffer.GetRemainLen() >=2)
        {
            if(HandleHead() < 0) return RETURN_ERROR;
            if(m_op_code == Op_Close) return RETURN_CLIENT_CLOSE;
        }



        if(m_parse_status == ParseExtendLength16 && m_frame_buffer.GetRemainLen() >=2)
        {
            m_payload_len = m_frame_buffer.GetUInt16();
            m_parse_status = ParseMaskKey;
        }
        else if(m_parse_status == ParseExtendLength64 && m_frame_buffer.GetRemainLen() >=8)
        {
            m_payload_len = m_frame_buffer.GetUInt64();
            m_parse_status = ParseMaskKey;
        }


        if(m_parse_status == ParseMaskKey && m_frame_buffer.GetRemainLen() >= 4)
        {
             //不能直接使用Mbuffer里的，如果mbuffer有relloc过，则指针的内容改变
            memcpy(m_mask_key,m_frame_buffer.GetBytes(4),4);
            m_parse_status = ParseData;
        }
    }
    return RETURN_NO;
}


void WebSocket::Reset()
{
    m_data_buffer.Clear();
    FramReset();
}

int WebSocket::Close(const char *message, int len)
{
    return Write(message,len,Op_Close);
}

int WebSocket::Write(const char *message, int len, unsigned char op_code)
{
    MBuffer *buffer = m_socket->GetNewBuffer();
    buffer->AddUInt8(0x80 | op_code);

    if(len < 126)
        buffer->AddUInt8(len);
    else if(len < 0xFFFF)
    {
        buffer->AddUInt8(126);
        buffer->AddUInt16(len);
    }
    else
    {
        buffer->AddUInt8(127);
        buffer->AddUInt64(len);
    }

    if(len > 0)
    {
         buffer->AddBytes(message,len);
    }
    m_socket->StartSend();
    return 0;
}

void WebSocket::FramReset()
{
    m_frame_buffer.Clear();

    m_is_finished = 0;
    m_op_code = Op_Close;
    m_payload_len = 0;
    m_is_mask = 0;

    m_parse_status = ParseHead;
}

int WebSocket::HandleHead()
{
    unsigned char first_char = m_frame_buffer.GetUInt8();
    unsigned char second_char = m_frame_buffer.GetUInt8();

    m_is_finished = first_char >> 7;
    m_op_code = first_char & 0xF;
    XXSERVER_LOG_INFO("opcode = %d\n",m_op_code);

    m_is_mask = second_char >> 7;
    m_payload_len = second_char & 0x7F;

    if(m_is_mask == 0)
    {
        XXSERVER_LOG_ERROR("mask != 1\n");
        return -1;
    }

    if(m_payload_len < 126)
        m_parse_status = ParseMaskKey;
    else if(m_payload_len == 126)
        m_parse_status = ParseExtendLength16;
    else if(m_payload_len == 127)
        m_parse_status = ParseExtendLength64;

    return 0;
}

int WebSocket::HandlePayloadData()
{
    unsigned char *data = (unsigned char *)m_frame_buffer.GetBytes(m_payload_len);
    for(int i = 0;i<m_payload_len;++i)
    {
        data[i] ^= m_mask_key[i&3];
    }
    m_data_buffer.AddBytes((char *)data,m_payload_len);
    return 0;
}

