#include "log.h"
#include "connmgr.h"
#include "proxymgr.h"
#include "datask.h"
#include "util.h"
#include "daacceptor.h"
#include "processor.h"
#include "datatimer.h"

bool g_run = true;
bool g_bSVCStatus = true;
StopperSignal* g_stopper = NULL;
u_int g_iTaskNum = 0;
u_int g_port = 0;
std::string g_strLocalAddr("127.0.0.1");

SVCMgr::SVCMgr(const string& strConf)
{
    LOG_INFO("<SVCMgr::SVCMgr> initialize enter... \n");
    ACE_Reactor_Impl* pDevPollReactor = new ACE_Dev_Poll_Reactor();

    if (pDevPollReactor != NULL)
    {
        ACE_Reactor::instance(new ACE_Reactor(pDevPollReactor));
    }

    ACE_Configuration_Heap config;
    if (config.open() == -1)
    {
        LOG_ERROR("<SVCMgr::SVCMgr> init config failed!\n");
        g_bSVCStatus = false;
    }
    
    ACE_Registry_ImpExp confImporter(config);
    if (confImporter.import_config(strConf.c_str()) == -1)
    {
        LOG_ERROR("<SVCMgr::SVCMgr> import config file failed!\n");
        g_bSVCStatus = false;
    }

    // 初始化日志服务
    std::string strLogPath;
    if (readConfValue(config, "log", "path", strLogPath) == false)
    {
        LOG_ERROR("<SVCMgr::SVCMgr> read config log path failed!\n");
        g_bSVCStatus = false;
    }

    string cmdline("-s ");
    cmdline.append(strLogPath);
    cmdline.append(" -m 40960 -N 5 -o -i 300 -f OSTREAM");

    if (init_logger(cmdline) == false)
    {
        LOG_ERROR("<SVCMgr::SVCMgr> init log failed!\n");
        g_bSVCStatus = false;
    }
    
    // 读取配置
    std::string strNetCard;
    std::string strRemoteAddrs;
    
    bool bflag = readConfValue(config, "localservice", "network_card", strNetCard);
    bflag = bflag&readConfValue(config, "localservice", "port", g_port);
    bflag = bflag&getLocalIP(strNetCard, g_strLocalAddr);
    bflag = bflag&readConfValue(config, "task", "thread_num", g_iTaskNum);
    bflag = bflag&readConfValue(config, "remoteservice", "svc_addr", strRemoteAddrs);

    if (bflag == false)
    {
        LOG_ERROR("<SVCMgr::SVCMgr> read config failed!\n");
        g_bSVCStatus = false;
    }
    
    // 连接后端服务
    addRemoteService(strRemoteAddrs);
    
}

SVCMgr::~SVCMgr()
{
    PROXYMGR::instance()->finalize();
    
    DAACCEPTOR::instance()->close();
    LOG_INFO("<SVCMgr::~SVCMgr> reactor task finish\n");

    // 停止服务
    g_run = false;

    for (int iNum = 0; iNum < g_iTaskNum; ++iNum)
    {
        DATASK::instance()->finalize();
    }
    LOG_INFO("<SVCMgr::~SVCMgr> server exit\n");
}

int SVCMgr::onDispatch(int iRouteKey, std::string& strMsg)
{
    int iRet = -1;
    int handle = PROXYMGR::instance()->getSvrConn(iRouteKey);
    if (handle != -1
        && CONNMGR::instance()->isExist(handle) == true)
    {
        iRet = CONNMGR::instance()->write(handle, strMsg);
        return iRet;
    }
    else
    {
        return iRet;
    }
}

// 请求响应方法
int SVCMgr::onResponse(int handle, std::string& strMsg)
{
    int iRet = -1;
    if (handle != -1
        && CONNMGR::instance()->isExist(handle) == true)
    {
        iRet = CONNMGR::instance()->write(handle, strMsg);
        return iRet;
    }
    else
    {
        return iRet;
    }
}

bool SVCMgr::onRegistProcessor(NetProcessor* pProc)
{
    return DATASK::instance()->setProc(pProc);
}

bool SVCMgr::onRegistTimer(TimerProcessor* pProc, const time_t& delay, const time_t& interval)
{
    if (pProc == NULL)
    {
        return false;
    }
    
    DataTimer* ptimer = new DataTimer(pProc, delay, interval);

    if (ptimer->getHandle() != -1 && 
        DATASK::instance()->addTimeProc(ptimer) == true)
    {
        return true;
    }
    
    delete ptimer;
    ptimer = NULL;
    return false;
}

bool SVCMgr::onRun()
{
    if (g_bSVCStatus == false)
    {
        return false;
    }
    
    // 启动服务
    if (g_iTaskNum > 0)
    {
        if (DATASK::instance()->activate(THR_NEW_LWP, g_iTaskNum) == -1)
        {
            LOG_ERROR("<SVCMgr::onRun> start datask failed %m\n");
            return false;
        }
    }

    // 初始化服务端口
    ACE_INET_Addr listen_addr(g_port, g_strLocalAddr.c_str());

    if (DAACCEPTOR::instance()->open(listen_addr, ACE_Reactor::instance(), 1) == -1)
    {
        LOG_ERROR("<SVCMgr::onRun> open acceptor failed %m\n");
        return false;
    }

    // 运行
    LOG_INFO("<SVCMgr::onRun> reactor task start\n");
    g_stopper = new StopperSignal();
    while (ACE_Reactor::instance()->reactor_event_loop_done() == 0)
    {
        ACE_Reactor::instance()->run_reactor_event_loop();
    }
    delete g_stopper;
    g_stopper = NULL;
    return true;
}
