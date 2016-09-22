#ifndef __SVRMGR_H
#define __SVRMGR_H

#include <stdint.h>
#include <time.h>
#include <vector>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include "ace/Singleton.h"
#include "ace/Task_T.h"
#include "util.h"

class SvrMgr
{
    public:
    
    typedef std::vector<int> HANDLE_LIST;
    typedef std::tr1::unordered_set<int> HANDLE_SET;
    
    enum STATUS
    {
        ST_NORMAL = 1,                          // ��������
        ST_UNNORMAL,                            // �����쳣
        ST_UNKNOWN = 255,
    };

    struct SServer
    {
        SServer() :nPos(0), eStat(ST_NORMAL), tVisitTime(0) 
        {}

        uint64_t nPos;  // ���˷�������
        STATUS eStat;  // ����server״̬
        time_t tVisitTime;  // ����յ�����ʱ��
        HANDLE_SET handles;  // ����server����session���� 
        HANDLE_LIST handle_list;
    };
    
    typedef std::tr1::unordered_map<PAIR_SVR_INFO, SServer, PairHash> MAP_SERVER;
    
        SvrMgr():m_mutex() {};
        
        ~SvrMgr(){};
        
        void addSvr(PAIR_SVR_INFO& pairSvrInfo);
        
        void rmSvr(PAIR_SVR_INFO& pairSvrInfo);
        
        void addSvrConn(uint32_t, uint16_t, int);
        
        void rmSvrConn(uint32_t, uint16_t, int);
        
        int connSvr(uint32_t, uint16_t);
        
        int getConn(const PAIR_SVR_INFO&);
        
        bool isSvrExist(const PAIR_SVR_INFO&);
        
        void closeSvr(PAIR_SVR_INFO& pairSvrInfo);
        
    private:
        ACE_RW_Thread_Mutex m_mutex;
        MAP_SERVER m_mapSvr;
};

typedef ACE_Unmanaged_Singleton<SvrMgr, ACE_Null_Mutex> SVRMGR;

#endif
