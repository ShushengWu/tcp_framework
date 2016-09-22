#ifndef __DASVCHANDLER_H
#define __DASVCHANDLER_H

#include <string>
#include "ace/SOCK_Stream.h"
#include "ace/Svc_Handler.h"

class DASvcHandler: public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
{
    typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;

    public:
        virtual int open(void* = 0);

        virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);   

        virtual int handle_output(ACE_HANDLE fd= ACE_INVALID_HANDLE); 

        virtual int handle_close(ACE_HANDLE handle, 
                                 ACE_Reactor_Mask close_mask);
                                 
        int write(ACE_Message_Block* msg);
        
        int write(std::string& strMsg);
                                 
    private:
        int isValidAndFinish(ACE_Message_Block* msg, int& iMsgSize) const;

    private:
        ACE_TCHAR m_peer_addr[BUFSIZ];
        ACE_Message_Block* m_phead;    // 标识读取内存起始位置
        ACE_Message_Block* m_pmsg;    // 标识读取内存当前位置
};

#endif
