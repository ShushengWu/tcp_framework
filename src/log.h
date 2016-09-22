#ifndef __LOG_H 
#define __LOG_H
#include "ace/Log_Msg.h"

using namespace std;

#ifndef LOG_PREFIX
#define LOG_PREFIX ACE_TEXT ("[%D|%P|%t|%M] ")
#endif

#ifndef LOG_DEBUG
#define LOG_DEBUG(...) \
        ACE_ERROR((LM_DEBUG, \
                   LOG_PREFIX __VA_ARGS__))
#endif

#ifndef LOG_INFO
#define LOG_INFO(...) \
        ACE_ERROR((LM_INFO, \
                   LOG_PREFIX __VA_ARGS__))
#endif

#ifndef LOG_WARNING
#define LOG_WARNING(...) \
        ACE_ERROR((LM_WARNING, \
                   LOG_PREFIX __VA_ARGS__))
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(...) \
        ACE_ERROR((LM_ERROR, \
                   LOG_PREFIX __VA_ARGS__))
#endif

class Trace
{
    public:
        Trace(const ACE_TCHAR* prefix,
              const ACE_TCHAR* name)
        {
            this->m_prefix = prefix;
            this->m_name = name;

            ACE_Log_Msg* lm = ACE_LOG_MSG;
            if (lm->tracing_enabled() && lm->trace_active() == 0)
            {
                lm->trace_active(1);
                ACE_DEBUG((LM_TRACE,
                           ACE_TEXT("%s enter %s {\n"),
                           this->m_prefix,
                           this->m_name));
                lm->trace_active(0);
            }
        }

        ~Trace()
        {
            ACE_Log_Msg* lm = ACE_LOG_MSG;
            if (lm->tracing_enabled() && lm->trace_active() == 0)
            {
                lm->trace_active(1);
                ACE_DEBUG((LM_TRACE,
                           ACE_TEXT("%s }exist %s\n"),
                           this->m_prefix,
                           this->m_name));
                lm->trace_active(0);
            }
        }

    private:
        const ACE_TCHAR* m_prefix;
        const ACE_TCHAR* m_name;
};

#ifndef LOG_TRACE
#if (ACE_NTRACE == 1)
#    define LOG_TRACE(X)
#else
#    define LOG_TRACE(X) \
#            Trace (LOG_PREFIX, X)
#endif
#endif

#endif

