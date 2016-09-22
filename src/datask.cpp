#include <iostream>
#include <string>
#include <sstream>
#include "constant.h"
#include "log.h"
#include "connmgr.h"
#include "proxymgr.h"
#include "processor.h"
#include "datatimer.h"
#include "datask.h"

using namespace std;
extern bool g_run;

DATask::DATask():m_mutex(), 
                 m_pProc(NULL),
                 m_mTProc()
{
}

DATask::~DATask()
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

int DATask::put(ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>* svc,
                ACE_Message_Block* mblk)
{
    int handle = svc->peer().get_handle();
    ACE_Message_Block* head = new ACE_Message_Block(sizeof(int));
    head->copy((char*)&handle, sizeof(int));
    head->cont(mblk);
    return this->putq(head);    
}

int DATask::svc(void)
{
    LOG_INFO("<DATask::svc> start\n");
    for (ACE_Message_Block* mblk; getq(mblk) != -1&&g_run;)
    {
        int handle = *(int*)mblk->rd_ptr();
        
        if (handle == TASK_EXIST_HANDLE)
        {
            LOG_INFO("<DATask::svc> cmd exist\n");
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
            mblk->release();
            continue;
        }
        
        // 再次确认是否请求连接是否存在，规避队列积压时请求断连
        if (CONNMGR::instance()->isExist(handle) == false)
        {
            LOG_WARNING("<DATask::svc> connection is not exist\n");
            mblk->release();
            continue;
        }
        
        ACE_Message_Block* pmsg = mblk->cont();
        process(handle, pmsg);
        mblk->release();
    }
    return 0;
}

bool DATask::process(int handle, ACE_Message_Block* pmsg)
{
    bool bRet = false;
    if (pmsg == NULL)
    {
        LOG_ERROR("<DATask::process> message is invalid!\n");
        return bRet;
    }
    
    if (m_pProc != NULL)
    {
        m_pProc->onProcessor(handle, pmsg->rd_ptr()+6,  pmsg->length()-7);
        bRet = true;
    }

    return bRet;
}

bool DATask::setProc(NetProcessor* pProc)
{
    if (m_pProc == NULL)
    {
        m_pProc = pProc;
        return true;
    }
    else
    {
        return false;
    }
}

bool DATask::addTimeProc(DataTimer* pTimer)
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

void DATask::finalize()
{
    LOG_INFO("<DATask::finalize> enter\n");
    int exithandle = TASK_EXIST_HANDLE;
    ACE_Message_Block* head = new ACE_Message_Block(sizeof(int));
    head->copy((char*)&exithandle, sizeof(int));
    int flag = putq(head);
    LOG_INFO("<DATask::finalize> exit %d\n", flag);
}

