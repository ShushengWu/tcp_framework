#ifndef __PROCESSOR_H
#define __PROCESSOR_H

// ��Ҫ�̳�NetProcessor��ʵ��onProcessor
// ��ܵ���onProcessor��������
class NetProcessor
{
    public:
    
    // ������������
    // input:
    //     handle:    ����˾��
    //     sMsg:    ������Ϣ��, ����Ҫ�����ͷ�ָ���ڴ��
    //     length:    ��Ϣ�峤��
    virtual void onProcessor(int handle, const char* sMsg, int length) = 0;
};

// ��Ҫ�̳�TimerProcessor��ʵ��onProcessor
// ��ܵ���onProcessor����ʱ��
class TimerProcessor
{
    public:
    
    // ��ʱ��������
    // input:
    //     iCurTime:    ��ʱ������ʱ��
    virtual void onProcessor(time_t iCurTime) = 0;
};

// ���������
class SVCMgr
{
    public:
        // ���������ļ�����·��
        SVCMgr(const string& strConf);
        
        ~SVCMgr();
        
        // ���������������ɷ��������������ɷ�����˷���
        static int onDispatch(int iRouteKey, std::string& strMsg);

        // ����������������Ӧ����
        static int onResponse(int handle, std::string& strMsg);

        // ����������ע���������������
        bool onRegistProcessor(NetProcessor* pProc);

        // ����������ע�����������
        bool onRegistTimer(TimerProcessor* pProc, const time_t& delay, const time_t& interval=0);

        // �������������з�����
        bool onRun();
};

#endif
