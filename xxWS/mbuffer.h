#ifndef MBUFFER_H
#define MBUFFER_H

class MBuffer
{

public:
    static bool IsBigEndian();

    MBuffer(int len = 1024);
    MBuffer(char *data,int len);
    ~MBuffer();

    void AddUInt8(unsigned char byte);
    unsigned char GetUInt8();

    void AddInt8(char c);
    char GetInt8();

    void AddInt16(short value);
    short GetInt16();

    void AddUInt16(unsigned short value);
    unsigned short GetUInt16();

    void AddInt32(int value);
    int GetInt32();

    void AddUInt32(unsigned int value);
    unsigned int GetUInt32();

    void AddInt64(long long value);
    long long GetInt64();

    void AddUInt64(unsigned long long value);
    unsigned long long GetUInt64();

    void AddBytes(const char *bytes,int len);
    void AddString(const char *s);

    char *GetBytes(int len);
    char *GetString();

    void Clear();
    void ReSet(int len = 0);
    void ResetToZero();

    char *GetBuffer(){return m_data;}
    int GetLen(){return m_offset;}

    int GetParseLen(){return m_parse_len;}

    char *GetRemainBytes(){return m_data+m_parse_len;}
    int GetRemainLen(){return m_offset - m_parse_len;}

private:
    void Extend(int len);
private:
    static char m_endian;

    char *m_data;
    int m_capacity;
    int m_offset;
    int m_parse_len;
    bool m_is_selfmem;
};

#endif // MBUFFER_H
