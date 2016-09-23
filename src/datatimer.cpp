#include "ace/OS_NS_time.h"
#include "log.h"
#include "datatimer.h"
#include "timertask.h"
#include "constant.h"
#include "processor.h"

DataTimer::DataTimer(TimerProcessor* pProc,
                   const time_t& delay, 
                   const time_t& interval):m_pProc(pProc)
{
    ACE_Time_Value tDelay(delay);
    ACE_Time_Value tInterval(interval);
    m_thandle = ACE_Reactor::instance()->schedule_timer(this, 
                                                           0, 
                                                           tDelay,
                                                           tInterval);
}

DataTimer::~DataTimer()
{
    if (m_thandle != -1)
    {
        ACE_Reactor::instance()->cancel_timer(m_thandle);
    }
    
    if (m_pProc != NULL)
    {
        delete m_pProc;
        m_pProc = NULL;
    }
}

int DataTimer::handle_timeout (const ACE_Time_Value& current_time,
                               const void* handle )
{
    time_t epoch = ((timespec_t)current_time).tv_sec - ACE_OS::timezone();
    
    int thandle = TASK_TIMER_HANDLE;
    ACE_Message_Block* head = new ACE_Message_Block(sizeof(int));
    head->copy((char*)&thandle, sizeof(int));

    ACE_Message_Block* hmsg = new ACE_Message_Block(sizeof(long));
    hmsg->copy((char*)&m_thandle, sizeof(long));
    ACE_Message_Block* tmsg = new ACE_Message_Block(sizeof(time_t));
    tmsg->copy((char*)&epoch, sizeof(time_t));
    
    hmsg->cont(tmsg);
    head->cont(hmsg);
    
    TIMERTASK::instance()->putq(head);
    
    LOG_INFO("<DataTimer::handle_timeout>handle_timeout: %d \n", m_thandle);
    return 0;
}

TimerProcessor* DataTimer::getProc() const
{
    return m_pProc;
}

long DataTimer::getHandle() const
{
    return m_thandle;
}
