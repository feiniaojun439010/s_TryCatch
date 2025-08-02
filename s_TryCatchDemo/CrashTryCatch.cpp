#include "CrashTryCatch.h"

const char* strsignal(int sig) {
    switch (sig) {
        case SIGINT:  return "Interrupt (Ctrl+C)";
        case SIGILL:  return "Illegal instruction";
        case SIGFPE:  return "Floating-point exception";
        case SIGSEGV: return "Segmentation violation";
        case SIGTERM: return "Termination request";
        case SIGABRT: return "Abnormal termination";
        default:      return "Unknown signal";
    }
}

thread_local int g_signum;
jmp_buf env;
void handle_crash(int signum)
{
    g_signum = signum;
#ifndef Q_OS_LINUX
    longjmp(env,1);
#else
    siglongjmp(env,1);
#endif
}

handle_excep<crash_error>& s_try_impl::operator +(handle_excep<crash_error>)
{
    handle_excep<crash_error>* obj = new handle_excep<crash_error>;
    obj->_sti = this;
    this->_finally_exce = obj;
    return *obj;
}

s_try_impl::~s_try_impl()
{
    if(_exce_fun)
    {
        signal(SIGINT,handle_crash);
        signal(SIGABRT,handle_crash);
        signal(SIGILL,handle_crash);
        signal(SIGFPE,handle_crash);
        signal(SIGSEGV,handle_crash);
        signal(SIGTERM,handle_crash);
#ifndef Q_OS_LINUX
        if(setjmp(env) == 0)
#else
        if(sigsetjmp(env,1) == 0)
#endif
        {
            try{
                _exce_fun();
            }
            catch(const std::exception& exce)
            {
                for(auto& e:_exce_vec)
                {
                    if(e->can_handle(exce))
                    {
                        e->handle(exce);
                    }
                }
            }
        }
        else
        {
#ifndef Q_OS_LINUX
            signal(SIGINT,handle_crash);
            signal(SIGABRT,handle_crash);
            signal(SIGILL,handle_crash);
            signal(SIGFPE,handle_crash);
            signal(SIGSEGV,handle_crash);
            signal(SIGTERM,handle_crash);
#endif
            if(_finally_exce)
            {
                const char* str = strsignal(g_signum);
                int len = strlen(str);
                std::string ss(str,len);
                _finally_exce->handle(crash_error{ss});
            }

        }
    }
    for(auto& it :_exce_vec)
    {
        delete it;
    }
    if(_finally_exce)
    {
        delete _finally_exce;
    }
}
