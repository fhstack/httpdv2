#pragma once

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"

template <class T>
class threadpool
{
public:
    threadpool(int thread_number = 8,int max_request = 8192);
    ~threadpool();

    //往请求队列中添加请求
    bool append(T* reaquest);
private:
    static void* worker(void* arg);

    void run();

private:
    //线程池维护的总线程数
    int m_thread_number;

    //请求队列中允许的最大请求数
    int m_max_request;

    //描述进程池的数组
    pthread_t* m_threads;

    //请求队列
    std::list<T*> m_workqueue;

    //保护请求队列的互斥量
    locker m_queuelocker;

    //当前是否有任务需要处理
    sem m_queuestat;

    //是否结束线程池
    bool m_stop;
};

template <class T>
threadpool<T>::threadpool(int thread_number ,int max_request )
    :m_thread_number(thread_number)
    ,m_max_request(max_request)
    ,m_stop(false)
    ,m_threads(NULL)
{
    if( ( m_thread_number <= 0 ) || (m_max_request <= 0) )
    {
        throw std::exception();
    }

    m_threads = new pthread_t[m_thread_number];

    if( m_threads == NULL)
    {
        throw std::exception();
    }
    //创建thread_number个线程，并将它们都设置为分离
    for(int i = 0;i < thread_number;i++)
    {
        if( pthread_create(m_threads+i,NULL,worker,this) != 0)
        {
            //commit it or roll back!
            delete[] m_threads;
            throw std::exception();
        }

        if(pthread_detach(m_threads[i]) != 0)
        {
            //commit it or roll back
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template <class T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
    m_stop = true;
}

template <class T>
bool threadpool<T>::append(T* request)
{
    printf("append %d \n",request->a);
    m_queuelocker.lock();
    
    if(m_workqueue.size() > m_max_request)
    {
        printf("append %d failed ,队列已满\n",request->a);
        m_queuelocker.unlock();
        return false;
    }

    m_workqueue.push_back(request);
    m_queuelocker.unlock();

    printf("%d append 成功\n",request->a);
    //信号量加一，可以运行的线程资源少1
    m_queuestat.post();
    return true;
}

//线程的工作函数
template <class T>
void* threadpool<T>::worker(void* arg)
{
    //this指针
    threadpool* pool = (threadpool*)arg;
    pool->run();
    return pool;
}

template <class T>
void threadpool<T>::run()
{
    while(!m_stop)
    {
        //首先信号量减1，如信号量已经为0则阻塞在此
        m_queuestat.wait();
        m_queuelocker.lock();
        //理论上工作队列不会为空，保守起见
        if(m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }

        //从工作对列中取出该任务
        T* request = m_workqueue.front();
        
        //删除该任务
        m_workqueue.pop_front();
        printf("执行 %d \n",request->a);       
        m_queuelocker.unlock();

        //理论上不会为空，保守起见
        if( request == NULL )
            continue;

        //printf("线程执行 process\n");
        //执行对应的任务后退出
        //request->process();
        
    }
}

