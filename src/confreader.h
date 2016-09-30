#ifndef __CONFREADER_H
#define __CONFREADER_H

#include <string>

class ACE_Configuration_Heap;

class ConfReader
{
    public:
        ConfReader(const std::string& strConfPath);
        ~ConfReader();
        
        bool readConfValue(const std::string& strSection,
                           const std::string& strName,
                           std::string& strValue);
                   
        bool readConfValue(const std::string& strSection,
                           const std::string& strName,
                           u_int& iValue);
        
    private:
        ACE_Configuration_Heap* m_pconfig;
};

#endif
