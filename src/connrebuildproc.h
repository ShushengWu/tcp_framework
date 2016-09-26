#ifndef __CONNREBUILDPROC_H
#define __CONNREBUILDPROC_H

#include "log.h"
#include "proxymgr.h"
#include "processor.h"

class ConnRebuildProc:public TimerProcessor
{
    virtual void onProcessor(time_t iCurTime)
    {
        PROXYMGR::instance()->reBuild();
    }
};

#endif
