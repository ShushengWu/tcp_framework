#ifndef __PROCESSOR_H
#define __PROCESSOR_H

// 需要继承NetProcessor并实现onProcessor
// 框架调用onProcessor处理请求
class NetProcessor
{
    public:
    
    // 网络请求处理方法
    // input:
    //     handle:    请求端句柄
    //     sMsg:    请求消息体，不需要负责释放指针内存块
    //     length:    消息体长度
    virtual void onProcessor(int handle, const char* sMsg, int length) = 0;
};

// 需要继承TimerProcessor并实现onProcessor
// 框架调用onProcessor处理定时器
class TimerProcessor
{
    public:
    
    // 定时器处理方法
    // input:
    //     iCurTime:    定时器触发时间
    virtual void onProcessor(time_t iCurTime) = 0;
};

// 服务管理类
class SVCMgr
{
    public:
        // 服务配置文件路径
        SVCMgr(const string& strConf);
        
        ~SVCMgr();
        
        // 被调方法，请求派发方法。将请求派发给后端服务
        static int onDispatch(int iRouteKey, std::string& strMsg);

        // 被调方法，请求响应方法
        static int onResponse(int handle, std::string& strMsg);

        // 被调方法，注册响应请求处理对象
        bool onRegistProcessor(NetProcessor* pProc);

        // 被调方法，注册定时器处理对象
        bool onRegistTimer(TimerProcessor* pProc, const time_t& delay, const time_t& interval=0);

        // 被调方法，运行服务框架
        bool onRun();
};

#endif
