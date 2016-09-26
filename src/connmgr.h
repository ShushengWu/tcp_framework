#ifndef __CONNMGR_H
#define __CONNMGR_H

#include <tr1/unordered_map>
#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h"
#include "daconnector.h"

// 保存连接，连接对象生命周期由acceptor和connector管理
class ConnMgr
{
    public:    
        ConnMgr();
        
        ~ConnMgr();

        void addConn(DASvcHandler*);
    
        void rmConn(DASvcHandler*);
        
        bool isExist(int);
        
        int connect(uint32_t, uint16_t);
        
        int write(int, std::string&);
        
        int close(int);
        
    private:
        ACE_RW_Thread_Mutex m_mutex;
        ACE_Recursive_Thread_Mutex m_rmutex;
        DAConnector m_connector;
        std::tr1::unordered_map<int, DASvcHandler*> m_mConn;        
};

typedef ACE_Unmanaged_Singleton<ConnMgr, ACE_Null_Mutex> CONNMGR;

#endif
