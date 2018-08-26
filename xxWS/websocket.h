#ifndef WEBSOCKET_H
#define WEBSOCKET_H
#include "mbuffer.h"


#define  Op_Text  1
#define  Op_Binary  2
#define  Op_Close  8
#define  Op_Ping  9
#define  Op_Pong  10


class XXSocket;
class WebSocket
{
    enum ParseStatus
    {
        ParseHead,
        ParseExtendLength16,
        ParseExtendLength64,
        ParseMaskKey,
        ParseData,
        ParseFinised
    };
public:
    WebSocket(XXSocket *_socket);

    int HandleData(const char *data,int len);
    void Reset();
    int Close(const char *message, int len);
    int Write(const char *message,int len,unsigned char op_code);
    bool GetIsFinished(){return  m_parse_status == ParseFinised;}

    unsigned char GetOpCode(){return m_op_code;}
    char *GetData(){return m_data_buffer.GetBuffer();}
    unsigned int GetLen(){return m_data_buffer.GetLen();}

    XXSocket *Socket(){return m_socket;}
private:
    void FramReset();
    int HandleHead();
    int HandlePayloadData();
private:
    MBuffer m_data_buffer;
    MBuffer m_frame_buffer;

    char m_is_finished;
    unsigned char m_op_code;
    unsigned int m_payload_len;
    char m_is_mask;

    unsigned char m_mask_key[4];
    ParseStatus m_parse_status;

    XXSocket *m_socket;
};

#endif // WEBSOCKET_H
