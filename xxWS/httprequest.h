#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <map>

class HttpRequest {
    struct char_comp
    {
        bool operator() (const char *a, const char *b) const
        {
            return strcmp(a,b)<0;
        }
    };
    typedef std::map<char *,char *,char_comp> StringMap;
    enum ParseStatus{ParsBegin,ParseHead,ParseBody,ParseFinised};

    ParseStatus m_pase_status;
    StringMap m_headers;
    int m_content_length;
    char *m_path;
    char *m_method;
    char *m_version;

    char *m_data;
    int m_parse_len;
    int m_offset;
    int m_capacity;
public:
    char *Path(){return m_path;}
    char *Method(){return m_method;}
    int ContentLen(){return m_content_length;}
    char *Data(){return m_data + m_parse_len;}
    bool IsParseFinished(){return m_pase_status == ParseFinised;}
    char *Header(const char *key)
    {
        StringMap::iterator it  = m_headers.find((char *)key);
        return it==m_headers.end()? NULL : it->second;
    }

    HttpRequest();
    ~HttpRequest();

    void ReSet();  //clear

    /* HandleData:
         * return -1 parse error
         * return 0 parse not finished
         * return 1 parse finished
         */
    int HandleData(char *buffer, int len);

    void Print();

private:
    char *Trim(char *&start,char *end);
    int ParseLine(char *line,char *end);
    int ParsePath(char *line);
    void Expand(int len);

};

#endif // HTTPREQUEST_H
