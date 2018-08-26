#include "mbuffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// 短整型大小端互换
#define Swap16(A)  ((((A) & 0xff00) >> 8) | \
    (((A) & 0x00ff) << 8))

// 长整型大小端互换
#define Swap32(A)  ((((A) & 0xff000000) >> 24) | \
    (((A) & 0x00ff0000) >> 8) | \
    (((A) & 0x0000ff00) << 8) | \
    (((A) & 0x000000ff) << 24))

// 64整型大小端互换
#define Swap64(A)  ((((A) & 0xff00000000000000) >> 56) | \
    (((A) & 0x00ff000000000000) >> 40) | \
    (((A) & 0x0000ff0000000000) >> 24) | \
    (((A) & 0x000000ff00000000) >> 8)) | \
    (((A) & 0x00000000ff000000) << 8) | \
    (((A) & 0x0000000000ff0000) << 24) | \
    (((A) & 0x000000000000ff00) << 40) | \
    (((A) & 0x00000000000000ff) << 56))

#define ADD_INT16(value)\
    if(!IsBigEndian())\
    value = Swap16(value);\
    AddBytes((const char *)&value,2);

#define GET_INT16(t) \
    t *p = (t *)(m_data+m_parse_len);\
    m_parse_len += 2;\
    return IsBigEndian() ? *p : Swap16(*p);

#define ADD_INT32(value)\
    if(!IsBigEndian())\
    value = Swap32(value);\
    AddBytes((const char *)&value,4);

#define GET_INT32(t) \
    t *p = (t *)(m_data+m_parse_len);\
    m_parse_len += 4;\
    return IsBigEndian() ? *p : Swap32(*p);

#define ADD_INT64(value)\
    if(!IsBigEndian())\
    value = Swap32(value);\
    AddBytes((const char *)&value,8);

#define GET_INT64(t) \
    t *p = (t *)(m_data+m_parse_len);\
    m_parse_len += 8;\
    return IsBigEndian() ? *p : Swap32(*p);


#define NULL 0

#define NO -1
#define LITLE 0
#define BIG 1

char MBuffer::m_endian = NO;
bool MBuffer::IsBigEndian()
{

    if(m_endian == NO)
    {
        unsigned int tmp = 1;
        if(*((char *)&tmp) == 1)
        {
            m_endian = LITLE;
        }else
        {
            m_endian = BIG;
        }
    }

    return m_endian;
}

MBuffer::MBuffer(int len)
{
    m_data = (char *)malloc(len);
    m_offset = 0;
    m_capacity = len;
    m_parse_len = 0;
    m_is_selfmem = true;
}

MBuffer::MBuffer(char *data, int len)
{
    m_data = data;
    m_offset = len;
    m_capacity = len;
    m_parse_len = 0;
    m_is_selfmem = false;
}

MBuffer::~MBuffer()
{
    if(m_is_selfmem)
    {
        free(m_data);
        m_data = NULL;
    }
}

void MBuffer::AddUInt8(unsigned char byte)
{
    Extend(1);
    ((unsigned char *)m_data)[m_offset++] = byte;
}

unsigned char MBuffer::GetUInt8()
{
    return ((unsigned char*)m_data)[m_parse_len++];
}

void MBuffer::AddInt8(char c)
{
    Extend(1);
    m_data[m_offset++] = c;
}

char MBuffer::GetInt8()
{
    return m_data[m_parse_len++];
}

void MBuffer::AddInt16(short value)
{
    ADD_INT16(value)
}

short MBuffer::GetInt16()
{
    GET_INT16(short)
}

void MBuffer::AddUInt16(unsigned short value)
{
    ADD_INT16(value)
}

unsigned short MBuffer::GetUInt16()
{
    GET_INT16(unsigned short)
}

void MBuffer::AddInt32(int value)
{
    ADD_INT32(value)
}

int MBuffer::GetInt32()
{
    GET_INT32(int)
}

void MBuffer::AddUInt32(unsigned int value)
{
    ADD_INT32(value)
}

unsigned int MBuffer::GetUInt32()
{
    GET_INT32(unsigned int)
}

void MBuffer::AddInt64(long long value)
{
    ADD_INT64(value)
}

long long MBuffer::GetInt64()
{
    GET_INT64(long long)
}

void MBuffer::AddUInt64(unsigned long long value)
{
    ADD_INT64(value)
}

unsigned long long MBuffer::GetUInt64()
{
    GET_INT64(unsigned long long)
}


void MBuffer::AddBytes(const char *bytes, int len)
{
    Extend(len);
    memcpy(m_data+m_offset,bytes,len);
    m_offset += len;

}

void MBuffer::AddString(const char *s)
{
    AddBytes(s,strlen(s) + 1);
}

char * MBuffer::GetBytes(int len)
{
    char *data = m_data+m_parse_len;
    m_parse_len += len;
    return data;
}

char *MBuffer::GetString()
{
    int len = strlen(m_data+m_parse_len) + 1;
    return GetBytes(len);
}

void MBuffer::Clear()
{
    m_parse_len = 0;
    m_offset = 0;
}

void MBuffer::ReSet(int len)
{
    m_parse_len = len;
}

void MBuffer::ResetToZero()
{
    m_data = (char *)memmove(m_data,m_data + m_parse_len,m_offset - m_parse_len);
    m_offset -= m_parse_len;
    m_parse_len = 0;
}




void MBuffer::Extend(int len)
{

    if(m_offset + len > m_capacity)
    {
        m_capacity = m_capacity + len + 32;
        m_data= (char *)realloc(m_data,m_capacity);
    }
}


