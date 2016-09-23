#include <iostream>
#include <string>
#include <sstream>
#include "constant.h"
#include "log.h"
#include "datatimer.h"
#include "processor.h"
#include "timertask.h"

extern bool g_run;

TimerTask::TimerTask():m_mutex(), 
                 m_mTProc()
{
}

TimerTask::~TimerTask()
{
    std::map<long, DataTimer*>::iterator itr = m_mTProc.begin();
    for (; itr != m_mTProc.end(); ++itr)
    {
        if (itr->second != NULL)
        {
            delete itr->second;
            itr->second = NULL;
        }
    }
}

int TimerTask::svc(void)
{
    LOG_INFO("<TimerTask::svc> start\n");
    for (ACE_Message_Block* mblk; getq(mblk) != -1&&g_run;)
    {
        int handle = *(int*)mblk->rd_ptr();
        
        if (handle == TASK_EXIST_HANDLE)
        {
            LOG_INFO("<TimerTask::svc> cmd exist\n");
            mblk->release();
            break;
        }
        
        if (handle == TASK_TIMER_HANDLE)
        {
            ACE_Message_Block* hmsg = mblk->cont();
            long thandle = *(long*)hmsg->rd_ptr();
            ACE_Message_Block* tmsg = hmsg->cont();
            time_t epoch = *(time_t*)tmsg->rd_ptr();
            
            if (m_mTProc.find(thandle) != m_mTProc.end())
            {
                m_mTProc[thandle]->getProc()->onProcessor(epoch);
            }
        }
        
        mblk->release();
    }
    return 0;
}

bool TimerTask::addTimeProc(DataTimer* pTimer)
{
    if (pTimer != NULL && pTimer->getHandle() != -1)
    {
        if (m_mTProc.find(pTimer->getHandle()) == m_mTProc.end())
        {
            m_mTProc[pTimer->getHandle()] = pTimer;
            return true;
        }
    }
    return false;
    
}

void TimerTask::finalize()
{
    LOG_INFO("<TimerTask::finalize> enter\n");
    int exithandle = TASK_EXIST_HANDLE;
    ACE_Message_Block* head = new ACE_Message_Block(sizeof(int));
    head->copy((char*)&exithandle, sizeof(int));
    int flag = putq(head);
    LOG_INFO("<TimerTask::finalize> exit %d\n", flag);
}
