#include "log.h"
#include "connmgr.h"
#include "svrmgr.h"

void SvrMgr::addSvr(PAIR_SVR_INFO& pairSvrInfo)
{
    ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
    MAP_SERVER::iterator itr = m_mapSvr.find(pairSvrInfo);
    if (itr == m_mapSvr.end())
    {
        m_mapSvr[pairSvrInfo] = SServer();
    }
    return;
} 

void SvrMgr::rmSvr(PAIR_SVR_INFO& pairSvrInfo)
{
    ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
    MAP_SERVER::iterator itrSvr = m_mapSvr.find(pairSvrInfo);
    if (itrSvr != m_mapSvr.end())
    {
        m_mapSvr.erase(itrSvr);
    }
    return;
}   

void SvrMgr::closeSvr(PAIR_SVR_INFO& pairSvrInfo)
{
    HANDLE_SET handles;
    do
    {
        ACE_Read_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
        MAP_SERVER::iterator itrSvr = m_mapSvr.find(pairSvrInfo);
        if (itrSvr != m_mapSvr.end())
        {
            handles = itrSvr->second.handles;
        }
        
    }while(false);
    HANDLE_SET::iterator itr = handles.begin();
    for (; itr != handles.end(); ++itr)
    {
        CONNMGR::instance()->close(*itr);
    }
    return;
}

void SvrMgr::addSvrConn(uint32_t ip, uint16_t port, int handle)
{
    ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
    PAIR_SVR_INFO pairSvrInfo = std::make_pair(ip, port);
    MAP_SERVER::iterator itr = m_mapSvr.find(pairSvrInfo);
    if (itr != m_mapSvr.end())
    {
        itr = m_mapSvr.find(pairSvrInfo);
        if (itr->second.handles.find(handle) != itr->second.handles.end())
        {
            return;
        }
        itr->second.handles.insert(handle);
        itr->second.handle_list.push_back(handle);
    }
    return;
}

void SvrMgr::rmSvrConn(uint32_t ip, uint16_t port, int handle)
{
    ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
    MAP_SERVER::iterator itr = m_mapSvr.find(std::make_pair(ip, port));
    if (itr != m_mapSvr.end())
    {
        HANDLE_SET::iterator itrHandle = itr->second.handles.find(handle);
        if (itrHandle != itr->second.handles.end())
        {
            itr->second.handles.erase(itrHandle);
            itr->second.handle_list.clear();
            itr->second.handle_list.resize(itr->second.handles.size());
            std::copy(itr->second.handles.begin(), itr->second.handles.end(), 
                itr->second.handle_list.begin());
        }
    }
    return;
}

int SvrMgr::connSvr(uint32_t ip, uint16_t port)
{
    return CONNMGR::instance()->connect(ip, port);
}

int SvrMgr::getConn(const PAIR_SVR_INFO& pairSvrInfo)
{
    ACE_Read_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
    int handle = -1;
    MAP_SERVER::iterator itrSvr = m_mapSvr.find(pairSvrInfo);
    if (itrSvr == m_mapSvr.end())
    {
        return handle;
    }
    if (itrSvr->second.eStat == ST_UNNORMAL)
    {
        // 后端连接异常，选择下一个可用连接
        return handle;
    }
            
    // 选择可用服务连接
    if (itrSvr != m_mapSvr.end() && itrSvr->second.handle_list.size() > 0)
    {
        size_t dwSize = itrSvr->second.handle_list.size();
        HANDLE_LIST::iterator itr = itrSvr->second.handle_list.begin()+(itrSvr->second.nPos++%dwSize);
        handle = *itr;
    }
    return handle;
}

bool SvrMgr::isSvrExist(const PAIR_SVR_INFO& pairSvrInfo)
{
    ACE_Read_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
    MAP_SERVER::iterator itrSvr = m_mapSvr.find(pairSvrInfo);
    if (itrSvr != m_mapSvr.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

int SvrMgr::getHandleNum(const PAIR_SVR_INFO& pairSvrInfo)
{
    ACE_Read_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
    MAP_SERVER::iterator itrSvr = m_mapSvr.find(pairSvrInfo);
    if (itrSvr != m_mapSvr.end())
    {
        return itrSvr->second.handle_list.size();
    }
    else
    {
        return -1;
    }
}
