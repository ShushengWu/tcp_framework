#ifndef __PROXYMGR_H
#define __PROXYMGR_H

#include <stdint.h>
#include <vector>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include "ace/Singleton.h"
#include "ace/Task_T.h"
#include "util.h"

class ProxyMgr
{
    public:  
    typedef std::vector<PAIR_SVR_INFO> VCT_SERVER; 
    typedef std::tr1::unordered_set<PAIR_SVR_INFO, PairHash> SET_SERVER;

    struct SProxy
    {
        SProxy() 
            : dwPos(0)
        {}

        uint64_t dwPos;
        SET_SERVER servers;
        VCT_SERVER server_list;
    };

    typedef std::tr1::unordered_map<int, SProxy> MAP_PROXY;

    ProxyMgr():m_mutex() {};
        
    ~ProxyMgr(){};
    
    bool addSvr(int, uint32_t, uint16_t);
    
    bool rmSvr(int, uint32_t, uint16_t);

    int getSvrConn(int);
    
    void finalize();

    private:
        ACE_RW_Thread_Mutex m_mutex;
        MAP_PROXY m_mapProxy;
};

typedef ACE_Unmanaged_Singleton<ProxyMgr, ACE_Null_Mutex> PROXYMGR;

#endif
