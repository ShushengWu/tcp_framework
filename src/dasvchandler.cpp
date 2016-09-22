#include "log.h"
#include "connmgr.h"
#include "datask.h"
#include "dasvchandler.h"

#define NTOH(len) ntohs(len)
#define HTON(len) htons(len)

const uint32_t MIN_MESSAGESIZE = 3*sizeof(char) + sizeof(uint32_t);
const uint32_t MAX_MESSAGESIZE = 67108864;    // 64M
const size_t INPUT_SIZE = 65536;    // 64K
const int MAX_SEND_NUM = 100;    // �����ϵ����100��

// data protocal : 0x02|cmd|length|data|0x03 (cmd-char, length-dw)
// length = MIN_MESSAGESIZE + len(data)

// -1 not valid;0 valid but not finished;1 valid and finished return length;
int DASvcHandler::isValidAndFinish(ACE_Message_Block* msg, int& iMsgSize) const
{  
    if (*(uint8_t*)msg->rd_ptr() != 0x02)
    {
        return -1;
    }
    
    if (msg->total_length() < MIN_MESSAGESIZE)
    {
        return -1;
    }
    
    iMsgSize = NTOH(*(uint32_t*)(msg->rd_ptr()+2));
    if (iMsgSize > MAX_MESSAGESIZE)
    {
        return -1;
    }
    
    if (iMsgSize > msg->total_length())
    {
        return 0;
    }
    
    if (*(uint8_t*)(msg->rd_ptr() + iMsgSize - 1) == 0x03)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

int DASvcHandler::open(void *p)
{
    if (super::open(p) == -1)
    {
        LOG_ERROR("<DASvcHandler::open> DA SVC handler open failed!%m\n");
        return -1;
    }
    
    m_phead = NULL;
    m_pmsg = NULL;
    
    // ��¼���ӶԶ�ip:port
    ACE_INET_Addr client_addr;
    this->peer().get_remote_addr (client_addr);
    client_addr.addr_to_string (m_peer_addr, sizeof m_peer_addr);
    LOG_INFO("<DASvcHandler::open> open new connect,peer:%s, %d\n", 
             m_peer_addr, this->peer().get_handle());
             
    CONNMGR::instance()->addConn(this);
    return 0;
}

int DASvcHandler::handle_input(ACE_HANDLE)
{
    // LOG_INFO("<DASvcHandler::handle_input> enter\n");
    ACE_Time_Value timeout(0, 600);
    // ��һ�θ����ӷ����ȡ�ڴ�
    if ( m_phead == NULL )
    {
        // ��һ�η���С�ڴ棬���ö��ζ�ȡ��ʽ����ճ��
        m_phead = new ACE_Message_Block(MIN_MESSAGESIZE);
        m_pmsg = m_phead;
    }

    do
    {
        ssize_t rest = this->peer().recv(m_pmsg->wr_ptr(), m_pmsg->space(), &timeout);
        if (rest > 0)
        {
            // ���ں˶�ȡ������
            LOG_DEBUG("<DASvcHandler::handle_input> recv %d from %s\n", 
                    rest, m_peer_addr);
            m_pmsg->wr_ptr( rest );
            int iLen = 0;
            int iRet = isValidAndFinish(m_phead, iLen);
            if (iRet == 0 && iLen > m_phead->total_size())
            {
                // ���ݰ���С���������ö�ȡ��������һ������������һ��������Ϣ
                ACE_Message_Block* msg = new ACE_Message_Block(iLen);
                msg->copy(m_pmsg->rd_ptr(), m_pmsg->length());
                m_phead->release();
                m_phead = msg;
                m_pmsg = m_phead;
            }
            else if (iRet == 1)
            {
                // ��ȡ������һ������
                // ÿ�δ���һ�����󣬽������ø���������
                // ���������ݴ��ݸ�Ӧ�ò㣬��Ӧ�ò��ͷ��ڴ�
                // ���³�ʼ��head�����±���������
                DATASK::instance()->put(this, m_phead);
                m_phead = new ACE_Message_Block(MIN_MESSAGESIZE);
                m_pmsg = m_phead;
                break;
            }
            else if (iRet == -1)
            {
                // ��ȡ���Ƿ�����,�˳��ر�����
                LOG_ERROR("<DASvcHandler::handle_input> recv invalid data peer:%s\n",
                        m_peer_addr);
                return -1;
            }
        }
        else if (rest <= 0)
        {
            if ( ACE_OS::last_error() == ETIME )
            {
                // �����쳣�ȴ��´λ���
                LOG_WARNING("<DASvcHandler::handle_input> recv error: %m peer:%s\n", 
                        m_peer_addr);
                return 0;
            }
            else
            {
                // �����쳣�˳�
                LOG_ERROR("<DASvcHandler::handle_input> recv error: %m close peer:%s\n",
                        m_peer_addr);
                return -1;
            }
        }
        
        // ��ʣ��ռ�,Э�������
        if ( m_pmsg->space() == 0)
        {
            LOG_ERROR("<DASvcHandler::handle_input> recv space error close peer:%s\n",
                        m_peer_addr);
            return -1;
        }
    }while(true);

    return 0;
}

int DASvcHandler::handle_output(ACE_HANDLE)
{
    ACE_Message_Block* mb;
    ACE_Time_Value nowait(ACE_OS::gettimeofday());
    int iCount = 0;
    while(-1 != this->getq(mb, &nowait))
    {
        ssize_t send_cnt = this->peer().send(mb->rd_ptr(), mb->length());
        if (send_cnt > 0)
        {
            mb->rd_ptr(send_cnt);
            LOG_DEBUG("<DASvcHandler::handle_output> send %d to %s\n", 
                    send_cnt, m_peer_addr);
        }
        else if (send_cnt <= 0)
        {
            if ( ACE_OS::last_error() == ETIME
                || ACE_OS::last_error() == EWOULDBLOCK
                || ACE_OS::last_error() == EAGAIN )
            {
                // �����쳣�ȴ��´λ���
                LOG_WARNING("<DASvcHandler::handle_output> send error: %m peer:%s\n", 
                        m_peer_addr);
                this->ungetq(mb);
                break;
            }
            else
            {
                // �����쳣�˳�
                LOG_ERROR("<DASvcHandler::handle_output> send error: %m close peer:%s\n",
                        m_peer_addr);
                return -1;
            }
        }

        if (mb->length() > 0)
        {
           // δ��������Ϣ�����·�����Ϣ����
            this->ungetq(mb);
        }
        else
        {
            // ��������Ϣ�ͷ��ڴ�
            mb->release();
        }
        
        ++iCount;
        if ( iCount > MAX_SEND_NUM )
        {
            break;
        }
    }

    // ������������ݣ�ע��epoll����CPU��
    if (this->msg_queue()->is_empty())
    {
        this->reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    }
    else
    {
        this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    }
    return 0;
}

int DASvcHandler::handle_close(ACE_HANDLE h, ACE_Reactor_Mask mask)
{
    LOG_INFO("<DASvcHandler::handle_close> close connect,peer:%s, %d\n", 
             m_peer_addr, this->peer().get_handle());
    CONNMGR::instance()->rmConn(this);
    // �˳��ͷ��ڴ�
    if (m_phead != NULL)
    {
        m_phead->release();
        m_phead = NULL;
    }

    return super::handle_close(h, mask);
}

int DASvcHandler::write(ACE_Message_Block* msg)
{
    if (this->putq(msg) == -1)
    {
        LOG_ERROR("<DASvcHandler::write> write message to peer:%s failed\n",
                        m_peer_addr);
        return -1;
    }
    this->reactor()->register_handler(this, ACE_Event_Handler::WRITE_MASK);
    return 0;
}

int DASvcHandler::write(std::string& strMsg)
{
    ACE_Message_Block* msg = new ACE_Message_Block(strMsg.length()+MIN_MESSAGESIZE);
    *msg->wr_ptr() = 0x02;    // begin-prefix
    msg->wr_ptr(1);
    *msg->wr_ptr() = 0x00;    // cmd
    msg->wr_ptr(1);
    uint32_t msg_size = HTON(3*sizeof(char) + sizeof(uint32_t) + 
                             (uint32_t)strMsg.length());
    *(uint32_t*)msg->wr_ptr() = msg_size;    // msg size
    msg->wr_ptr(4);    
    ACE_OS::memcpy(msg->wr_ptr(), strMsg.c_str(), strMsg.length());
    msg->wr_ptr(strMsg.length());
    *msg->wr_ptr() = 0x03;    // end-prefix
    msg->wr_ptr(1);
    return write(msg);
}
