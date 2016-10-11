#include "log.h"
#include "svrmgr.h"
#include "proxymgr.h"

bool ProxyMgr::addSvr(int key, uint32_t ip, uint16_t port)
{
    PAIR_SVR_INFO pairSvrInfo = std::make_pair(ip, port);

    ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
    MAP_PROXY::iterator itr = m_mapProxy.find(key);
    if (itr == m_mapProxy.end())
    {
        m_mapProxy[key] = SProxy();
    }
    itr = m_mapProxy.find(key);
    if (itr->second.servers.find(pairSvrInfo) != itr->second.servers.end())
    {
        LOG_ERROR("<ProxyMgr::addSvr>key exist\n");
        return false;
    }
    
    if (SVRMGR::instance()->isSvrExist(pairSvrInfo))
    {
        LOG_ERROR("<ProxyMgr::addSvr>server exist\n");
        return false;
    }
    SVRMGR::instance()->addSvr(pairSvrInfo);
    SVRMGR::instance()->connSvr(ip, port);
    itr->second.servers.insert(pairSvrInfo);
    itr->second.server_list.push_back(pairSvrInfo);
    return true;
}

bool ProxyMgr::rmSvr(int key, uint32_t ip, uint16_t port)
{
    PAIR_SVR_INFO pairSvrInfo = std::make_pair(ip, port);
    ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
    MAP_PROXY::iterator itr = m_mapProxy.find(key);
    if (itr == m_mapProxy.end())
    {
        return true;
    }
    
    SET_SERVER::iterator itrSvr = itr->second.servers.find(pairSvrInfo);
    if (itrSvr == itr->second.servers.end())
    {
        return true;
    }
    SVRMGR::instance()->closeSvr(pairSvrInfo);
    SVRMGR::instance()->rmSvr(pairSvrInfo);
    itr->second.servers.erase(itrSvr);
    
    itr->second.server_list.clear();
    itr->second.server_list.resize(itr->second.servers.size());
    std::copy(itr->second.servers.begin(), itr->second.servers.end(), itr->second.server_list.begin());
    return true;
    
}

void ProxyMgr::finalize()
{
    ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
    MAP_PROXY::iterator itr = m_mapProxy.begin();
    for (; itr != m_mapProxy.end(); ++itr)
    {
        VCT_SERVER& servers = itr->second.server_list;
        VCT_SERVER::iterator itrSvr = servers.begin();
        for (;itrSvr != servers.end(); ++itrSvr)
        {
            SVRMGR::instance()->closeSvr(*itrSvr);
            SVRMGR::instance()->rmSvr(*itrSvr);
        }
    }
}

int ProxyMgr::getSvrConn(int routeKey)
{
    int handle = -1;
    MAP_PROXY::iterator itrProxy = m_mapProxy.find(routeKey);
    if ( itrProxy != m_mapProxy.end() )
    {
        size_t nSvrSize = itrProxy->second.server_list.size();
        for ( size_t inum = 0; inum < nSvrSize; ++inum )
        {
            VCT_SERVER::iterator itr = 
                itrProxy->second.server_list.begin();//+(++(itrProxy->second.dwPos)%nSvrSize);;
            ptrdiff_t nPos = ++(itrProxy->second.dwPos)%nSvrSize;
            handle = SVRMGR::instance()->getConn(*(itr+nPos));
            if (handle == -1)
            {
                // 后端服务异常，选择下一个可用服务
                continue;
            }
        }
    }
    return handle;
}

void ProxyMgr::reBuild()
{
    ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
    MAP_PROXY::iterator itr = m_mapProxy.begin();
    for (; itr != m_mapProxy.end(); ++itr)
    {
        VCT_SERVER& servers = itr->second.server_list;
        VCT_SERVER::iterator itrSvr = servers.begin();
        for (;itrSvr != servers.end(); ++itrSvr)
        {
            int iNum = SVRMGR::instance()->getHandleNum(*itrSvr);
            if (iNum == 0)
            {
                // 重建连接
                SVRMGR::instance()->connSvr(itrSvr->first, itrSvr->second);
            }
            else if (iNum > 3)
            {
                // 删除过多连接
                for (int iCount = 0; iCount < iNum - 3; ++iCount)
                {
                    int handle = SVRMGR::instance()->getConn(*itrSvr);
                    if ( handle != -1)
                    {
                        SVRMGR::instance()->rmSvrConn(itrSvr->first, itrSvr->second, handle);
                    }
                }
            }
        }
    }
}


void ProxyMgr::updateProxy(std::tr1::unordered_map<int, SET_SERVER>& mapSvrs)
{
    // 新增路由
    std::tr1::unordered_map<int, SET_SERVER>::iterator itrMap = mapSvrs.begin();
    for (; itrMap != mapSvrs.end(); ++itrMap)
    {
        SET_SERVER::iterator itrSvr = itrMap->second.begin();
        for (; itrSvr != itrMap->second.end(); ++itrSvr)
        {
            addSvr(itrMap->first, itrSvr->first, itrSvr->second);
        }
    }
    
    // 删除不存在的路由
    ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
    MAP_PROXY::iterator itr = m_mapProxy.begin();
    for (; itr != m_mapProxy.end();)
    {
        if ( mapSvrs.find(itr->first) == mapSvrs.end() )
        {
            // 删除全量路由
            SET_SERVER::iterator itrSvr = itr->second.servers.begin();
            for (; itrSvr != itr->second.servers.end(); ++ itrSvr)
            {
                SVRMGR::instance()->closeSvr(*itrSvr);
                SVRMGR::instance()->rmSvr(*itrSvr);
            }
            itr = m_mapProxy.erase(itr);
        }
        else
        {
            // 删除部分路由
            SET_SERVER::iterator itrSvr = itr->second.servers.begin();
            for (; itrSvr != itr->second.servers.end(); )
            {
                if (mapSvrs[itr->first].find(*itrSvr) == mapSvrs[itr->first].end())
                {
                    SVRMGR::instance()->closeSvr(*itrSvr);
                    SVRMGR::instance()->rmSvr(*itrSvr);
                    itrSvr = itr->second.servers.erase(itrSvr);
                }
                else
                {
                    ++itrSvr;
                }
            }
            itr->second.server_list.clear();
            itr->second.server_list.resize(itr->second.servers.size());
            std::copy(itr->second.servers.begin(), itr->second.servers.end(), itr->second.server_list.begin());
            ++itr;
        }
    }
}