#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <exception>
#include <string>
#include <type_traits>
#include <functional>
#ifndef Q_OS_LINUX
#include <windows.h>
#endif


struct crash_error : std::exception
{
    crash_error(std::string err) : _err{err}{}
    const char* what() const noexcept override
    {
        return _err.c_str();
    }
private:
    std::string _err;
};

struct handle_excep_base;
template<typename T,typename = std::enable_if_t<std::is_base_of<std::exception,T>::value
                                                ||std::is_same<std::exception,T>::value
                                                >>
struct handle_excep;
struct s_try_impl
{
   s_try_impl() = default;
   ~s_try_impl();
   s_try_impl(const s_try_impl&) = delete;
   s_try_impl& operator=(const s_try_impl&) = delete;
   s_try_impl(s_try_impl&& sti)
   {
       _exce_fun = std::move(sti._exce_fun);
       _exce_vec = std::move(sti._exce_vec);
       _finally_exce = sti._finally_exce;
       sti._finally_exce = nullptr;

   }
   s_try_impl& operator =(s_try_impl&& sti)
   {
       _exce_fun = std::move(sti._exce_fun);
       _exce_vec = std::move(sti._exce_vec);
       _finally_exce = sti._finally_exce;
       sti._finally_exce = nullptr;
       return *this;

   }
   s_try_impl& operator +(std::function<void()> fun)
   {
       _exce_fun = std::move(fun);
       return *this;

   }
   template<typename T>
   handle_excep<T>& operator +(handle_excep<T>);

   handle_excep<crash_error>& operator +(handle_excep<crash_error>);


private:
   std::function<void()> _exce_fun;
   std::vector<handle_excep_base*> _exce_vec;
   handle_excep_base* _finally_exce{nullptr};
};

struct handle_excep_base
{
    virtual bool can_handle(const std::exception& exce) noexcept = 0;
    virtual void handle(const std::exception& exce) noexcept = 0;
};
template<typename T>
struct handle_excep_impl : handle_excep_base
{
    bool can_handle(const std::exception& exce) noexcept override
    {
        auto r = dynamic_cast< const T*>(&exce);
        return r !=nullptr;
    }
    void handle(const std::exception& exce) noexcept override
    {
        const auto& r = dynamic_cast<const T&>(exce);
        if(_fun)
            _fun(r);
    }
    std::function<void(const T&)> _fun;
};

template<typename T,typename>
struct handle_excep: handle_excep_impl<T>
{
    s_try_impl& operator +(std::function<void(const T&)> fun)
    {
        this->_fun = fun;
        return *_sti;
    }
    s_try_impl* _sti;
};



template<typename T>
handle_excep<T>& s_try_impl::operator +(handle_excep<T>)
{
    handle_excep<T>* obj = new handle_excep<T>;
    _exce_vec.push_back(obj);
    obj->_sti = this;
    return *obj;
}


#define s_try s_try_impl()+[&]()
#define s_catch(exce_t,nm)+handle_excep<exce_t>{}+[](const exce_t nm)



