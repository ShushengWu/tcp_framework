#ifndef __DATATIMER_H
#define __DATATIMER_H

#include "ace/Event_Handler.h"

class TimerProcessor;

class DataTimer: public ACE_Event_Handler
{
    public:

        DataTimer(TimerProcessor* pProc,
                   const time_t& delay, 
                   const time_t& interval=0);
                   
        ~DataTimer();

        int handle_timeout (const ACE_Time_Value& current_time,
                            const void* = 0 );
        
        TimerProcessor* getProc() const;
        
        long getHandle() const;
    private:
        long m_thandle;
        TimerProcessor* m_pProc;

};

#endif

