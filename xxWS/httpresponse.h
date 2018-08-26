#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>
#include <map>
using namespace std;

class XXSocket;
class HttpResponse
{
    typedef map<string,string> StringMap;
public:
    HttpResponse(XXSocket *_socket);

    int WriteBodyData(const char *buf,int len);
    int Response404();
    int Response400();
    void SetHeader(string key, string value);
    void Reset();
    void Close();

    int ResponseWebsocket(const char *auth);
    int ResponseData(const char *data, int len);

    XXSocket *Socket(){return m_socket;}
private:
    XXSocket *m_socket;
    StringMap m_header_map;
    string m_res_data;
};

#endif // HTTPRESPONSE_H
