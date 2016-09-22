#include <iostream>
#include <sstream>
#include <string>
#include <ace/OS.h>
#include <ace/INET_Addr.h>
#include "log.h"
#include "util.h"
#include "proxymgr.h"
#include "util.h"

bool readConfValue(ACE_Configuration_Heap& config,
                   const std::string& strSection,
                   const std::string& strName,
                   std::string& strValue)
{
    ACE_Configuration_Section_Key section;
    if (config.open_section(config.root_section(), strSection.c_str(), 0, section) == -1)
    {
        LOG_ERROR("<readConfValue> read config %s section failed!\n", strSection.c_str());
        return false;
    }

    ACE_TString tstrValue;
    if (config.get_string_value(section, strName.c_str(), tstrValue) == -1)
    {
        LOG_ERROR("<readConfValue> read config %s failed!\n", strName.c_str());
        return false;
    }

    strValue = tstrValue.c_str();
    return true;
}

bool readConfValue(ACE_Configuration_Heap& config,
                   const std::string& strSection,
                   const std::string& strName,
                   u_int& iValue)
{
    ACE_Configuration_Section_Key section;
    if (config.open_section(config.root_section(), strSection.c_str(), 0, section) == -1)
    {
        LOG_ERROR("<readConfValue> read config %s section failed!\n", strSection.c_str());
        return false;
    }

    u_int value;
    if (config.get_integer_value(section, strName.c_str(), value) == -1)
    {
        LOG_ERROR("<readConfValue> read int config %s, %s failed!\n", strSection.c_str(), strName.c_str());
        return false;
    }
    iValue = value;

    return true;
}

bool getLocalIP(const std::string& strEthName, std::string& strIP)
{
    ACE_HANDLE handle = ACE_OS::socket(AF_INET, SOCK_STREAM, 0);
    if (handle == ACE_INVALID_HANDLE)
    {
        LOG_ERROR("<GetLocalIP> invalid handle!\n");
        return false;
    }

    ifreq if_address;
    ACE_OS::strncpy (if_address.ifr_name, strEthName.c_str(), sizeof(if_address.ifr_name));
    if (ACE_OS::ioctl (handle, SIOCGIFADDR, &if_address) == -1)
    {
        LOG_ERROR("<GetLocalIP> get interface failed!\n");
        ACE_OS::close (handle);
        return false;
    }

    sockaddr_in *socket_address = reinterpret_cast<sockaddr_in*> (&if_address.ifr_addr);
    strIP = ACE_OS::inet_ntoa(socket_address->sin_addr);
    ACE_OS::close (handle);
    return true;
}

bool addRemoteService(const std::string& strRemoteAddrs)
{
    LOG_INFO("<addRemoteService>%s\n", strRemoteAddrs.c_str());
    std::stringstream ss(strRemoteAddrs);
    std::string strItem;
    while (std::getline(ss, strItem, ';'))
    {
        std::size_t pos = 0;
        std::size_t tpos = strItem.find_first_of('*', pos);
        std::string strKey = strItem.substr(pos, tpos - pos);
        std::string strAddrs = strItem.substr(tpos+1, strItem.length() - tpos -1);

        std::stringstream ssKey;
        ssKey << strKey;
        int iRouteKey;
        ssKey >> iRouteKey;
        
        std::stringstream ssAddr(strAddrs);
        std::string strAddr;
        while(std::getline(ssAddr, strAddr, ','))
        {
            ACE_INET_Addr addr(strAddr.c_str());
            PROXYMGR::instance()->addSvr(iRouteKey, addr.get_ip_address(),
                                 addr.get_port_number());
            LOG_INFO("<addRemoteService>connect:%s, key:%d\n", strAddr.c_str(), iRouteKey);
            strAddr.clear();
        }
        
        strItem.clear();
    }
    return true;
}
