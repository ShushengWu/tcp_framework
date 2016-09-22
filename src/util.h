#ifndef __UTIL_H
#define __UTIL_H
#include "ace/Dev_Poll_Reactor.h"
#include "ace/Reactor.h"
#include "ace/Configuration.h"
#include "ace/Configuration_Import_Export.h"

class StopperSignal: public ACE_Event_Handler
{
    public:
        StopperSignal(int signum = SIGINT)
        {
            ACE_Reactor::instance()->register_handler(signum, this);
        }

        ~StopperSignal()
        {
            ACE_Reactor::instance()->remove_handler(this, SIGINT);
        }

        virtual int handle_signal(int signum, siginfo_t* = 0, ucontext_t* = 0)
        {
            ACE_Reactor::instance()->end_reactor_event_loop();
        }
};

bool readConfValue(ACE_Configuration_Heap& config,
                   const std::string& strSection,
                   const std::string& strName,
                   std::string& strValue);
                   
bool readConfValue(ACE_Configuration_Heap& config,
                   const std::string& strSection,
                   const std::string& strName,
                   u_int& iValue);
                  
bool getLocalIP(const std::string& strEthName, std::string& strIP);

bool addRemoteService(const std::string& strRemoteAddrs);

class PairHash
{
    public:
        uint64_t operator() (const std::pair<uint32_t, uint16_t> &k) const
        {
            return ( ((uint64_t)k.first<<32u)|(uint64_t)k.second );
        }
};
    
typedef std::pair<uint32_t, uint16_t> PAIR_SVR_INFO;

template<typename T> class Proxy
{
    public:
        Proxy(T* obj):m_proxy(obj)
        {}

        ~Proxy()
        {
            if (m_proxy != NULL)
            {
                delete m_proxy;
                m_proxy = NULL;
            }
        }
    private:
        T* m_proxy;
};

bool init_logger(string&);
bool enable_debug();
bool disable_debug();

#endif
