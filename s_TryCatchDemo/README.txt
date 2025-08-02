程序崩溃时可以通过捕获到signal使程序跳转到预设的恢复点运行，利用这种机制，我封装了s_try和s_catch宏。
其原理是借助c++的try catch跳转功能以及成员回调函数，设置的自己分支跳转功能，同时也能够捕获try catch的异常，构造了crash_error结构体，利用strsignal函数解析程序崩溃原因传入给s_catch
使得用户可以获取crash原因，使用方式如下
    s_try{
        int* a = nullptr;
        int b = *a;
    }
    s_catch(crash_error,e)
    {
	std::cout << e.what() << std::end;
    };
s_try的上一行代码存储程序指针，跳转到 s_catch执行后程序继续向下执行。