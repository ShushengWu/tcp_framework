#ifndef __MYSQLCONN_H
#define __MYSQLCONN_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <my_global.h>
#include <mysql.h>

class MysqlResult;

class MysqlConn
{
    public:
        MysqlConn(const std::string& strDBHost, const std::string& strDBUser, 
                  const std::string& strDBPwd, const std::string& strDBName, int iDBPort);
        ~MysqlConn();
        
        bool isValid();
        
        bool query(std::string& strSQL, MysqlResult& myResult);
    private:
        MYSQL m_conn;
        MYSQL* m_pconn;
        std::string m_strHost;
        std::string m_strUser;
        std::string m_strPwd;
        std::string m_strDbName;
        int m_iPort;
};

class MysqlResult
{
    public:
        MysqlResult();
        MysqlResult(MYSQL_RES*);
        ~MysqlResult(){};
        
        bool next();
        
        bool getColumn();
        
        template<typename T> bool getColumn(int index, T& value)
        {
            if (index <= m_iNumFields && m_rows[m_iCurRow-1][index] != NULL)
            {
                std::stringstream ss;
                ss << m_rows[m_iCurRow-1][index];
                ss >> value;
                ss.clear();
                return true;
            }
            return false;
        }
        
    private:
        int m_iNumFields;
        vector<MYSQL_ROW> m_rows;
        int m_iCurRow;
        int m_iRowNum;
};

#endif
