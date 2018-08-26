#ifndef EVENT_H
#define EVENT_H


class EventManager;
class Event
{
public:
    Event(EventManager *mgr):m_event_mgr(mgr),m_fd(-1){}
    virtual ~Event(){}
    virtual int HandleRead(){return 0;}
    virtual int HandleWrite(){return 0;}
    virtual int HandleError(){return 0;}
    int GetFd(){return m_fd;}
    void SetFd(int fd){m_fd = fd;}

	void SetEvent(int e) { m_event = e; }
	int GetEvent() { return m_event; }
protected:
    int m_fd;
	int m_event;
    EventManager *m_event_mgr;
};

#endif // POLLHANDLE_H
