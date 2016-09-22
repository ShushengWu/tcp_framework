#ifndef __DACONNECTOR_H
#define __DACONNECTOR_H

#include "ace/Connector.h"
#include "ace/SOCK_Connector.h"
#include "dasvchandler.h"

typedef ACE_Connector<DASvcHandler, ACE_SOCK_CONNECTOR> DAConnector;

#endif
