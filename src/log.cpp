#include <string>
#include "ace/ARGV.h"
#include "ace/Service_Config.h"
#include "ace/Get_Opt.h"
#include "ace/Logging_Strategy.h"
#include "ace/Dynamic_Service.h"
#include "ace/Service_Config.h"
#include "log.h"
#include "util.h"

using namespace std;

ACE_Logging_Strategy* plog;

bool init_logger(string& cmd)
{
    plog = ACE_Dynamic_Service<ACE_Logging_Strategy>::instance("Logger");

    if (plog == NULL)
    {
        int iRet = ACE_Service_Config::process_directive(
                       ACE_DYNAMIC_SERVICE_DIRECTIVE("Logger",
                                                     "ACE",
                                                     "_make_ACE_Logging_Strategy",
                                                     ""));
        LOG_INFO("ACE_Service_Config opened! result=%d\n", iRet);
    }

    plog = ACE_Dynamic_Service<ACE_Logging_Strategy>::instance("Logger");

    if (plog == NULL)
    {
        LOG_ERROR("ACE Logger open failed! %m\n");
        return false;
    }

    ACE_ARGV cmdline_args(cmd.c_str());
    if (plog->init(cmdline_args.argc(), cmdline_args.argv()))
    {
        LOG_ERROR("ACE Logger initialize failed! %m\n");
        return false;
    }
    ACE_LOG_MSG->priority_mask(0, ACE_Log_Msg::PROCESS);
    ACE_LOG_MSG->priority_mask(LM_INFO|LM_WARNING|LM_ERROR, 
                               ACE_Log_Msg::PROCESS);
    LOG_INFO("ACE Logger initialize succeded!\n");
    return true;
}

bool enable_debug()
{
    ACE_LOG_MSG->priority_mask(0, ACE_Log_Msg::PROCESS);
    ACE_LOG_MSG->priority_mask(LM_TRACE|LM_DEBUG|LM_INFO|LM_WARNING|LM_ERROR,
                               ACE_Log_Msg::PROCESS);
    return true;
}

bool disable_debug()
{
    ACE_LOG_MSG->priority_mask(0, ACE_Log_Msg::PROCESS);
    ACE_LOG_MSG->priority_mask(LM_INFO|LM_WARNING|LM_ERROR,
                               ACE_Log_Msg::PROCESS);
    return true;
}

