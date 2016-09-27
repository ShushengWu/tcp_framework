#ifndef __ParsePkg_H
#define __ParsePkg_H

#include <string>

// data protocal:0x02|len|data|0x03 (len-w)
class ParsePkg
{
    enum {
        PKG_HEAD=0X02,
        PKG_TAIL=0X03,
    };
    enum {
        MIN_PKG_SIZE=(unsigned short)0x04,
        MAX_PKG_SIZE=(unsigned short)0xffff,
    };

public:
    ParsePkg(){};
    ~ParsePkg(){};

    // -1 not valid;0 valid but not finished;1 valid and finished return length;
    int isValidAndFinish(const char* pMsg, const size_t nMsgSize, size_t& nRealMsgLen)
    {
        int iRet = -1;
        // 判断数据大小
        if (nMsgSize < 0)
        {
            return iRet;
        }
        else if (nMsgSize == 0)
        {
            return 0;
        }

        // 判断包头
        if (nMsgSize < MIN_PKG_SIZE)
        {
            if (*pMsg != PKG_HEAD)
            {
                return -1;
            }
            return 0;
        }
        else
        {
            if (*pMsg != PKG_HEAD)
            {
                return -1;
            }
        }

        nRealMsgLen = ntohs(*((unsigned short*)(pMsg+1)));
        // 判断协议长度
        if (nRealMsgLen > MAX_PKG_SIZE || nRealMsgLen < MIN_PKG_SIZE)
        {
            return -1;
        }
        if (nMsgSize >= nRealMsgLen)
        {
            // 判断包尾
            if (*(pMsg+nRealMsgLen-1) == PKG_TAIL)
            {
                iRet = 1;
            }
            else
            {
                iRet = -1;
            }
        }
        else
        {
            iRet = 0;
        }
        return iRet;
    }

    std::string mkMsgHead(const std::string& strMsg)
    {
        std::string strHead = std::string(MIN_PKG_SIZE-1, 0x00);
        *((char*)strHead.data()) = PKG_HEAD; 
        *(uint16_t*)(strHead.data()+1) = htons(strMsg.length()+MIN_PKG_SIZE);
        return strHead;
    }

    std::string mkMsgTail(const std::string& strMsg)
    {
        std::string strTail = std::string(1, PKG_TAIL);
        return strTail;
    }

    const char* getMsgData(const char* pMsg, const size_t nSize)
    {
        if (*pMsg != PKG_HEAD && *(pMsg+nSize-1) != PKG_TAIL)
        {
            return NULL;
        }
        return pMsg+MIN_PKG_SIZE-1;
    }

    size_t getMsgLen(const char* pMsg, const size_t nSize)
    {
    	if (nSize < MIN_PKG_SIZE)
    	{
    	    return -1;
    	}

        if (*pMsg != PKG_HEAD && *(pMsg+nSize-1) != PKG_TAIL)
        {
            return -1;
        }
        return ntohs(*((unsigned short*)(pMsg+1)));
    }

    size_t min_msg_size()
    {
        return MIN_PKG_SIZE;
    }
};

#endif
