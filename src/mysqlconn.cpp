#include <iostream>
#include "log.h"
#include "mysqlconn.h"

MysqlConn::MysqlConn(const std::string& strDBHost, const std::string& strDBUser, 
                     const std::string& strDBPwd, const std::string& strDBName, int iDBPort)
                     :m_strHost(strDBHost), m_strUser(strDBUser), m_strPwd(strDBPwd),
                     m_strDbName(strDBName), m_iPort(iDBPort)
{
    mysql_init(&m_conn);
    MYSQL* m_pconn = mysql_real_connect(&m_conn, m_strHost.c_str(),
        m_strUser.c_str(), m_strPwd.c_str(), m_strDbName.c_str(),
        m_iPort, NULL, 0);
    if (m_pconn == NULL)
    {
        LOG_ERROR("<MysqlConn::MysqlConn>failed to connect to database: %s\n", 
            mysql_error(&m_conn));
    }
}

MysqlConn::~MysqlConn()
{
    mysql_close(&m_conn);
}

bool MysqlConn::isValid()
{
    if (m_pconn != NULL)
    {
        return true;
    }
    return false;
}

bool MysqlConn::query(std::string& strSQL, MysqlResult& myResult)
{
    if (mysql_query(&m_conn, strSQL.c_str()) != 0)
    {
        LOG_ERROR("<MysqlConn::query>failed to read from database: %s\n",
            mysql_error(&m_conn));
        return false;
    }
    MYSQL_RES* presult = mysql_store_result(&m_conn);
    if (presult == NULL)
    {
        LOG_ERROR("<MysqlConn::query>failed to store result: %s\n",
            mysql_error(&m_conn));
        return false;       
    }

    MysqlResult result(presult);
    myResult = result;
    mysql_free_result(presult);
    return true;
}

MysqlResult::MysqlResult():m_iNumFields(0), m_iCurRow(0), 
                         m_iRowNum(0), m_rows()
{
}

MysqlResult::MysqlResult(MYSQL_RES* presult)
                        :m_iNumFields(0), m_iCurRow(0), 
                         m_iRowNum(0), m_rows()
{
    if (presult != NULL)
    {
        m_iNumFields = mysql_num_fields(presult);
        MYSQL_ROW row;
        while((row = mysql_fetch_row(presult)))
        {
            if (row[0] == NULL)
            {
                continue;        
            } 
            m_rows.push_back(row);
        }
        m_iRowNum = m_rows.size();
    }
}

bool MysqlResult::next()
{
    bool bFlag = true;
    ++m_iCurRow;
    if ( m_iCurRow > m_iRowNum )
    {
        bFlag = false;
    }
    return bFlag;
}

