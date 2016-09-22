#ifndef __DATASK_H
#define __DATASK_H

#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h"
#include "ace/Task_T.h"
#include <map>

class NetProcessor;
class DataTimer;

class DATask: public ACE_Task<ACE_MT_SYNCH>
{
    public:

        DATask();

        ~DATask();

        int put(ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>*, 
                ACE_Message_Block*);

        virtual int svc(void);

        void finalize();
        
        bool setProc(NetProcessor* );
        
        bool addTimeProc(DataTimer*);

    private:
        bool process(int, ACE_Message_Block*);

    private:
        ACE_RW_Thread_Mutex m_mutex;

        std::map<int, int> m_mreq;
        
        NetProcessor* m_pProc;
        std::map<long, DataTimer*> m_mTProc;
};

typedef ACE_Unmanaged_Singleton<DATask, ACE_Null_Mutex> DATASK;

#endif

