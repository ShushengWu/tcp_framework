#ifndef __TIMERTASK_H
#define __TIMERTASK_H

#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h"
#include "ace/Task_T.h"
#include <map>

class DataTimer;

class TimerTask: public ACE_Task<ACE_MT_SYNCH>
{
    public:

        TimerTask();

        ~TimerTask();

        virtual int svc(void);

        void finalize();
        
        bool addTimeProc(DataTimer*);

    private:
        ACE_RW_Thread_Mutex m_mutex;

        std::map<long, DataTimer*> m_mTProc;
};

typedef ACE_Unmanaged_Singleton<TimerTask, ACE_Null_Mutex> TIMERTASK;

#endif
