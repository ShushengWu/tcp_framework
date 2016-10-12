#include <iostream>
#include <string>
#include <sstream>
#include "constant.h"
#include "log.h"
#include "connmgr.h"
#include "processor.h"
#include "parsepkg.h"
#include "datask.h"

using namespace std;
extern bool g_run;
extern ParsePkg* g_parser;

DATask::DATask():m_mutex(), 
                 m_pProc(NULL)
{
}

DATask::~DATask()
{
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
        if (mblk->msg_type() == ACE_Message_Block::MB_STOP)
        {
            LOG_INFO("<DATask::svc> exist\n");
            mblk->release();
            break;
        }
        int handle = *(int*)mblk->rd_ptr();
        
        // 再次确认请求连接是否存在，规避队列积压时请求断连
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
    
    if (m_pProc != NULL&&g_parser != NULL)
    {
        int len = g_parser->getDataLen(pmsg->rd_ptr(),  pmsg->length());
        if (len != -1)
        {
            m_pProc->onProcessor(handle, g_parser->getMsgData(pmsg->rd_ptr(),  pmsg->length()), len);
            bRet = true;
        }
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

void DATask::finalize()
{
    LOG_INFO("<DATask::finalize> enter\n");
    ACE_Message_Block* head = new ACE_Message_Block(0, ACE_Message_Block::MB_STOP);
    int flag = putq(head);
    LOG_INFO("<DATask::finalize> exit %d\n", flag);
}

