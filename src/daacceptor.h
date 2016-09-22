#ifndef __DAACCEPTOR_H
#define __DAACCEPTOR_H

#include "ace/Singleton.h"
#include "ace/Acceptor.h"
#include "ace/SOCK_Acceptor.h"
#include "dasvchandler.h"

typedef ACE_Acceptor<DASvcHandler, ACE_SOCK_ACCEPTOR> DAAcceptor;

typedef ACE_Unmanaged_Singleton<DAAcceptor, ACE_Null_Mutex> DAACCEPTOR;

#endif
