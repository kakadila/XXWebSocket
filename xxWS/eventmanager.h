#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#ifdef WIN32

#define USE_SELECT //使用select

#define EV_WRITE 0x1
#define EV_READ 0x10
#define EV_ERROR 0x100

#include <map>
#include <vector>
#else
#include <sys/epoll.h>
#define USE_EPOLL

#define EPOLL_INIT_SIZE 256
#define EPOLL_EVENT_SIZE 128
#define EPOLL_WAIT_TIME 1000

#define EV_WRITE EPOLLOUT
#define EV_READ EPOLLIN
#define EV_ERROR EPOLLERR
#endif

#include "net_common.h"
#include "event.h"

#define EVENT_WAIT_TIME 1000 //事件循环超时时间毫秒

class EventManager
{
public:
    EventManager();
    ~EventManager();

    int Open();
    int AddEvent(Event *ev,unsigned int e);
    int ModEvent(Event *ev,unsigned int e);
    int RemoveEvent(Event *ev);

    int StartLoop();
    void Stop();
private:
    int m_handle;

#ifdef USE_EPOLL
    epoll_event m_epoll_events[EPOLL_EVENT_SIZE];
#endif

#ifdef USE_SELECT
	typedef std::map<int, Event *> FdMap;
	FdMap m_fd_map;
	fd_set m_read_set;
	fd_set m_write_set;
	fd_set m_error_set;
    std::vector<int> m_remove_list;
#endif
    bool m_run;
};

#endif // POLLMANAGER_H
