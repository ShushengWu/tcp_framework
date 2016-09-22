#include <iostream>
#include "log.h"
#include "processor.h"

class RealProcessor:public TimerProcessor
{
    virtual void onProcessor(time_t iCurTime)
    {
        for (int iNum = 1; iNum < 100; ++iNum)
        {
            std::string strMsg(iNum, 't');
            SVCMgr::onDispatch(1, strMsg);
        } 
    }
};

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        LOG_ERROR("<main> args is invalide! do: ./proc proc.conf\n");
        return false;
    }

    SVCMgr svc = SVCMgr(argv[1]);

    for (int iNum = 1; iNum < 100; ++iNum)
    {
        std::string strMsg(iNum, 't');
        SVCMgr::onDispatch(1, strMsg);
    }

    TimerProcessor* pProc = new RealProcessor();
    
    svc.onRegistTimer(pProc, 10);
    
    svc.onRun();
    
    if (pProc != NULL)
    {
        delete pProc;
        pProc = NULL;
    }
    return 0;
}
