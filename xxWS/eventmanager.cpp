#include "eventmanager.h"
#include <stdio.h>


EventManager::EventManager()
{
    m_handle = -1;
}

EventManager::~EventManager()
{
    if(m_handle > 0)
        close(m_handle);
    m_handle = -1;
}

void EventManager::Stop()
{
	m_run = false;
}

#ifdef USE_EPOLL
int EventManager::Open()
{
	m_run = true; 
	m_handle = epoll_create(EPOLL_EVENT_SIZE); 
	return m_handle;
}

int EventManager::AddEvent(Event *ev, unsigned int e)
{
	epoll_event e_event; 
	e_event.events = e; 
	e_event.data.ptr = (void *)ev; 
	ev->SetEvent(e); 
	return epoll_ctl(m_handle, EPOLL_CTL_ADD, ev->GetFd(), &e_event);
}

int EventManager::ModEvent(Event *ev, unsigned int e)
{
	epoll_event e_event; 
	e_event.events = e; 
	e_event.data.ptr = (void *)ev; 
	ev->SetEvent(e); 
	return epoll_ctl(m_handle, EPOLL_CTL_MOD, ev->GetFd(), &e_event);
}

int EventManager::RemoveEvent(Event *ev)
{
    epoll_ctl(m_handle, EPOLL_CTL_DEL, ev->GetFd(), NULL);
    close(ev->GetFd());
    ev->SetFd(-1);
    return 0;
}

int EventManager::StartLoop()
{
	m_run = true;
	int count; 
	while (m_run) 
	{ 
		count = epoll_wait(m_handle, m_epoll_events, EPOLL_EVENT_SIZE, EPOLL_WAIT_TIME); 
		for (int i = 0; i<count; ++i) 
		{ 
			epoll_event &e_event = m_epoll_events[i]; 
			Event *ev = (Event*)e_event.data.ptr; 
			if (e_event.events & EV_READ) 
				ev->HandleRead(); 
			else if (e_event.events & EV_WRITE) 
				ev->HandleWrite(); 
			else if (e_event.events & EV_ERROR) 
				ev->HandleError(); 
		} 
	}
    return 0;
}
#endif


#ifdef USE_SELECT

int EventManager::Open()
{
	FD_ZERO(&m_read_set); 
	FD_ZERO(&m_write_set); 
	FD_ZERO(&m_error_set); 
	return 0;
}

int EventManager::AddEvent(Event *ev, unsigned int e)
{
	int fd = ev->GetFd(); 
	m_fd_map[fd] = ev; 
	if (e & EV_READ) 
		FD_SET(fd, &m_read_set); 
	if (e & EV_WRITE) 
		FD_SET(fd, &m_write_set); 
	FD_SET(fd, &m_error_set); 
	ev->SetEvent(e); 
	return 0;
}

int EventManager::ModEvent(Event *ev, unsigned int e)
{
    int fd = ev->GetFd();
    FdMap::iterator it = m_fd_map.find(fd);
    if (it == m_fd_map.end()) return 0;
    int old_e = ev->GetEvent();
    if(old_e == e) return 0;

    if (old_e & EV_READ)
        FD_CLR(fd, &m_read_set);
    if (old_e & EV_WRITE)
        FD_CLR(fd, &m_write_set);
    if (e & EV_READ)
        FD_SET(fd, &m_read_set);
    if (e & EV_WRITE)
        FD_SET(fd, &m_write_set);
    it->second->SetEvent(e);

	return 0;
}

int EventManager::RemoveEvent(Event *ev)
{
	//加入删除列表
    m_remove_list.push_back(ev->GetFd());
    ev->SetFd(-1);
	return 0;
}


int EventManager::StartLoop()
{
	m_run = true;
	int count;
	TIMEVAL tm;
	tm.tv_sec = 0;
    tm.tv_usec = EVENT_WAIT_TIME * 1000;

	fd_set read_set, write_set, error_set;
	
	while (m_run)
	{
		read_set = m_read_set;
		write_set = m_write_set;
		error_set = m_error_set;
		count = select(0, &read_set, &write_set, &error_set, &tm);
		if (count == -1)
		{
            XXSERVER_LOG_INFO("select error!%d\n",WSAGetLastError());
			break;
		}
			
		else if (count == 0)
		{
            //XXSERVER_LOG_INFO("timeout!\n");
			continue;
		}

		//响应事件
		for (FdMap::iterator it = m_fd_map.begin();
			it != m_fd_map.end(); ++it)
		{	
			int fd = it->first;
			Event *ev = it->second;
			int e = ev->GetEvent();
			if ( e & EV_READ && FD_ISSET(fd,&read_set))
				ev->HandleRead();
			if (e & EV_WRITE && FD_ISSET(fd, &write_set))
				ev->HandleWrite();
			if (FD_ISSET(fd, &error_set))
				ev->HandleError();		
		}

		//将删除列表的文件描述符删除 
		int size = m_remove_list.size(); 
		for (int i = 0; i < size; ++i)
		{
			int fd = m_remove_list[i];
			m_fd_map.erase(fd);
			FD_CLR(fd, &m_read_set);
			FD_CLR(fd, &m_write_set);
			FD_CLR(fd, &m_error_set);
            close(fd);
		}
		m_remove_list.clear();
	}
	return 0;
}
#endif
