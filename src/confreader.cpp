#include "ace/Configuration.h"
#include "ace/Configuration_Import_Export.h"
#include "log.h"
#include "confreader.h"

ConfReader::ConfReader(const std::string& strConfPath)
{
    m_pconfig = new ACE_Configuration_Heap();
    if (m_pconfig->open() == -1)
    {
        LOG_ERROR("<ConfReader::ConfReader> init config failed!\n");
        delete m_pconfig;
        m_pconfig = NULL;
    }
    
    ACE_Registry_ImpExp confImporter(*m_pconfig);
    if (confImporter.import_config(strConfPath.c_str()) == -1)
    {
        LOG_ERROR("<ConfReader::ConfReader> import config file failed!\n");
        delete m_pconfig;
        m_pconfig = NULL;
    }
}

ConfReader::~ConfReader()
{
    if (m_pconfig != NULL)
    {
        delete m_pconfig;
        m_pconfig = NULL;
    }
}

bool ConfReader::readConfValue(const std::string& strSection,
                           const std::string& strName,
                           std::string& strValue)
{
    if (m_pconfig == NULL)
    {
        return false;
    }
    ACE_Configuration_Section_Key section;
    if (m_pconfig->open_section(m_pconfig->root_section(), strSection.c_str(), 0, section) == -1)
    {
        LOG_ERROR("<readConfValue> read config %s section failed!\n", strSection.c_str());
        return false;
    }

    ACE_TString tstrValue;
    if (m_pconfig->get_string_value(section, strName.c_str(), tstrValue) == -1)
    {
        LOG_ERROR("<readConfValue> read config %s failed!\n", strName.c_str());
        return false;
    }

    strValue = tstrValue.c_str();
    return true;
}

bool ConfReader::readConfValue(const std::string& strSection,
                           const std::string& strName,
                           u_int& iValue)
{
    if (m_pconfig == NULL)
    {
        return false;
    }
    ACE_Configuration_Section_Key section;
    if (m_pconfig->open_section(m_pconfig->root_section(), strSection.c_str(), 0, section) == -1)
    {
        LOG_ERROR("<readConfValue> read config %s section failed!\n", strSection.c_str());
        return false;
    }

    u_int value;
    if (m_pconfig->get_integer_value(section, strName.c_str(), value) == -1)
    {
        LOG_ERROR("<readConfValue> read int config %s, %s failed!\n", strSection.c_str(), strName.c_str());
        return false;
    }
    iValue = value;

    return true;
}
