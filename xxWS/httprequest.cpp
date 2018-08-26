#include "httprequest.h"

#ifdef WIN32
#define strtok_r strtok_s
#endif

HttpRequest::HttpRequest()
{
    m_capacity = 1024;
    m_data = (char *)malloc(m_capacity);
    ReSet();
}

HttpRequest::~HttpRequest()
{
    free(m_data);
}

void HttpRequest::ReSet()
{
    m_headers.clear();
    m_pase_status = ParsBegin;
    m_content_length = 0;
    m_parse_len = 0;
    m_offset = 0;
}

int HttpRequest::HandleData(char *buffer, int len)
{
    Expand(len);

    memcpy(m_data+m_offset,buffer,len);
    m_offset += len;
    char *start = m_data+m_parse_len;
    m_data[m_offset] = 0;

    while(len > 0)
    {
        len = m_offset - m_parse_len;
        if(m_pase_status == ParseBody)
        {
            if(len >= m_content_length)
            {
                m_pase_status = ParseFinised;
                return 1;
            }
            else return 0;
        }

        char *end = strchr(start,'\n');
        int cur_parse_len = end - start + 1;
        if(end > start && *(end - 1) == '\r')
        {
            *end = *(end-1) =  0;
            m_parse_len += cur_parse_len;
            if(m_pase_status == ParseHead && ParseLine(start,end) < 0) return -1;
            if(m_pase_status == ParsBegin && ParsePath(start) <0 ) return -1;

        }else if(end != NULL)
            return -1;
        else return 0;

        start = end+1;
    }

    return 0;
}

void HttpRequest::Print()
{
    printf("=====================\n");
    printf("method : %s\n",m_method);
    printf("path : %s\n",m_path);
    printf("version : %s\n",m_version);
    for(StringMap::iterator it = m_headers.begin();
        it != m_headers.end();++it)
    {
        printf("%s:%s\n",it->first,it->second);
    }
    printf("content_len = %d\n",m_content_length);
    if(m_content_length > 0)
    {
        printf("data : %s\n",m_data + m_parse_len);
    }
}

char *HttpRequest::Trim(char *&start, char *end)
{
    while(*start <= 32 && *start > 0) *(start ++ ) = 0;
    while(*end <= 32 && *end > 0) *(end -- ) = 0;
    return start;
}

int HttpRequest::ParseLine(char *line, char *end)
{
    if(*line == 0){
        m_pase_status = ParseBody;
        return 0;
    }

    char *value = NULL;
    char *key = strtok_r(line,":",&value);
    if(key == NULL || *value == 0) return -1;
    Trim(key,value -2);
    Trim(value,end);

    if(strcmp(key,"Content-Length")== 0) m_content_length = atoi(value);
    m_headers[key] = value;

    return 0;
}

int HttpRequest::ParsePath(char *line)
{
    m_method = strtok_r(line," \t",&m_path);
    if(m_method == NULL || *m_path == 0) return -1;

    m_path = strtok_r(m_path," \t",&m_version);
    if(m_path == NULL || *m_path != '/') return -1;

    m_pase_status = ParseHead;
    return 0;
}

void HttpRequest::Expand(int len)
{
    if(len + m_offset > m_capacity)
    {
        m_capacity = m_capacity + len ;
        char *new_data  = (char *)realloc(m_data,m_capacity);
        //如果mbuffer有relloc过，则指针的内容改变
        if(new_data != m_data)
        {
            m_method = new_data + (m_method- m_data);
            m_version = new_data +(m_version - m_data);
            m_path   = new_data + (m_path - m_data);
            for(StringMap::iterator it = m_headers.begin();
                it!=m_headers.end();++it)
            {
                (char *&)(it->first) = new_data + (it->first - m_data);
                it->second = new_data + (it->second - m_data);
            }
        }
        m_data = new_data;
    }
}
