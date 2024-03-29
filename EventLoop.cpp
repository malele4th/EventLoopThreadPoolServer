/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#include "EventLoop.h"
#include<ctime>
#include<cstdio>

void printCurrentTime()
{
    time_t rawtime;
    struct tm *ptminfo;
    time(&rawtime);
    ptminfo = localtime(&rawtime);
    printf("==== %02d-%02d-%02d %02d:%02d:%02d",
		ptminfo->tm_year+1900,ptminfo->tm_mon+1,ptminfo->tm_mday,
		ptminfo->tm_hour,ptminfo->tm_min,ptminfo->tm_sec);
}

class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        signal(SIGPIPE, SIG_IGN);
    }
};

// 忽略SIGPIPE信号
IgnoreSigPipe initObj;

EventLoop::EventLoop()
    :isLooping(false),
     threadId(CurrentThread::gettid()),
     isQuit(false),
     e(new Epoll(this))
{ }

EventLoop::~EventLoop()
{
    assert(!isLooping);
}

void EventLoop::loop()
{
    assert(!isLooping);
    isLooping = true;
    isQuit = false;

    while(!isQuit)
    {
	printCurrentTime();
        std::cout << "---Looping---"<<std::endl;
        addToLoop();
        std::vector<Handler*> activeEvents;
        activeEvents.clear();
        // 调用epoll将活动的套接字描述符对应的Handler取出并处理
        e->epoll(activeEvents);
        for(std::vector<Handler*>::iterator iter = activeEvents.begin();
            iter != activeEvents.end(); ++iter)
        {
            std::cout << "----------Handle request----------" << std::endl;
            // 处理客户端请求的入口
            (*iter)->handle();
            e->removeFd((*iter)->connFd());
            delete *iter;
        }
    }
    isLooping = false;
}

void EventLoop::quit()
{
    isQuit = true;
}

// 每次将一个套接字描述符添加到描述符数组中
void EventLoop::addToLoop(const int fd)
{
    // e->addToEpoll(fd);
    std::cout << "----------Add " << fd << " to loop----------" << std::endl;
    fds.push_back(fd);
}

// 将描述符数组中的所有套接字全部添加到epoll中
void EventLoop::addToLoop()
{
    if(fds.empty())
    {   
	printCurrentTime();
        std::cout << "---fds empty---"<<std::endl;
        return;
    }

    std::cout<<"----------fds.size() = "<<fds.size() <<"----------"<<std::endl;
    for(int i = 0; i < fds.size(); ++i)
        e->addToEpoll(fds[i]);
    fds.clear();
    std::cout << "----------Add all fd to loop----------" << std::endl;
}
