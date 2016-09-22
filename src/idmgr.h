#ifndef __IDMGR_H
#define __IDMGR_H

#include "ace/Thread_Mutex.h"
#include "ace/Condition_T.h"
#include "ace/Singleton.h"

class IDMgr
{

    public:
        IDMgr():m_id(0) {}
        ~IDMgr() {}

        int getID()
        {
            ACE_Guard<ACE_Thread_Mutex> guard(m_mutex);
            if (m_id > 65535)
            {
                m_id = 0;
            }
            return m_id++;
        }

    private:
        ACE_Thread_Mutex m_mutex;
        int m_id;
};

typedef ACE_Unmanaged_Singleton<IDMgr, ACE_Null_Mutex> IDMGR;

#endif

