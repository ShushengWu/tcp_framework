#include "log.h"
#include "svrmgr.h"
#include "connmgr.h"

ConnMgr::ConnMgr():m_mutex(), m_rmutex(), m_connector()
{}

ConnMgr::~ConnMgr()
{
   m_connector.close();
}

void ConnMgr::addConn(DASvcHandler* svc)
{
    int handle = svc->peer().get_handle();
    do
    {
        ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
        if (m_mConn.find(handle) == m_mConn.end())
        {
            m_mConn[handle] = svc;
        }
    }while(false);

    ACE_INET_Addr peer_addr;
    svc->peer().get_remote_addr(peer_addr);
    SVRMGR::instance()->addSvrConn(peer_addr.get_ip_address(),
                                   peer_addr.get_port_number(),
                                   handle);
    return;
}

void ConnMgr::rmConn(DASvcHandler* svc)
{
    int handle = svc->peer().get_handle();
    do
    {
        ACE_Guard<ACE_Recursive_Thread_Mutex> rguard(m_rmutex);
        ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
        if (m_mConn.find(handle) != m_mConn.end())
        {
            m_mConn.erase(handle);
        }
    }while(false);
    
    ACE_INET_Addr peer_addr;
    svc->peer().get_remote_addr(peer_addr);
    SVRMGR::instance()->rmSvrConn(peer_addr.get_ip_address(),
                                   peer_addr.get_port_number(),
                                   handle);
    return;
}

bool ConnMgr::isExist(int handle)
{
    do
    {
        ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
        if (m_mConn.find(handle) != m_mConn.end())
        {
            return true;
        }
    }while(false);
    return false;
}

int ConnMgr::write(int handle, std::string& strMsg)
{
    int iRet = 0;
    do
    {
        ACE_Read_Guard<ACE_RW_Thread_Mutex> guard(m_mutex);
        if (m_mConn.find(handle) != m_mConn.end())
        {
            iRet = m_mConn[handle]->write(strMsg);
        }
        else
        {
            iRet = -2;
        }
    }while(false);
    return iRet;
}

int ConnMgr::close(int handle)
{
    int iRet = 0;
    do
    {
        ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_rmutex);
        if (m_mConn.find(handle) != m_mConn.end())
        {
            iRet = m_mConn[handle]->close();
        }
        else
        {
            iRet = -2;
        }
    }while(false);
    return iRet;
}

int ConnMgr::connect(uint32_t ip, uint16_t port )
{
    ACE_INET_Addr listen_addr(port, ip);
    DASvcHandler* pclient = NULL;
    if (m_connector.connect(pclient, listen_addr) == -1)
    {
        LOG_ERROR("<ConnMgr::connect> open connector failed %m\n");
        return -1;
    }
    return 0;
}
