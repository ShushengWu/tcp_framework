#include <iostream>
#include "log.h"
#include "processor.h"

class RealProcessor:public NetProcessor
{
    virtual void onProcessor(int handle, const char* sMsg, int length)
    {
        std::cout << "handle:" << handle << "\t msg:" << std::string(sMsg+3, length-4) << std::endl; 
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

    NetProcessor* pProc = new RealProcessor();
    
    svc.onRegistProcessor(pProc);
    
    svc.onRun();
    
    if (pProc != NULL)
    {
        delete pProc;
        pProc = NULL;
    }
    return 0;
}
