#pragma once

#include "error_levels.hpp"
#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"

#include "iceoryx_hoofs/error_handling_2/platform/error_reporting.hpp"

#include <atomic>
#include <iostream>
#include <mutex>

namespace eh
{
// interface is defined by the error levels and not completely fixed
// NB: we could have a switch on the levels a levels enum which would basically dispatch to the registered functions
//     for each level (one for each because they could be vastly different)

struct HandlerInterface
{
    virtual void operator()(const SourceLocation& location, Fatal, error_code_t code, module_id_t module) = 0;
    virtual void operator()(const SourceLocation& location, Error, error_code_t code, module_id_t module) = 0;
    virtual void operator()(const SourceLocation& location, Warning, error_code_t code, module_id_t module) = 0;

    virtual void preterminate() = 0;
};

struct DefaultHandler : public HandlerInterface
{
    void operator()(const SourceLocation& location, Fatal, error_code_t code, module_id_t module)
    {
        (void)location;
        (void)code;
        (void)module;
    }

    void operator()(const SourceLocation& location, Error, error_code_t code, module_id_t module)
    {
        (void)location;
        (void)code;
        (void)module;
    }

    void operator()(const SourceLocation& location, Warning, error_code_t code, module_id_t module)
    {
        (void)location;
        (void)code;
        (void)module;
    }

    void preterminate()
    {
        std::cout << "DEFAULT HANDLER WILL TERMINATE!" << std::endl;
    }
};

struct CountingHandler : public HandlerInterface
{
    // using fatal_handler_t = void (*)(const SourceLocation&, error_code_t, module_id_t);
    // using error_handler_t = void (*)(const SourceLocation&, error_code_t, module_id_t);
    // using warning_handler_t = void (*)(const SourceLocation&, error_code_t, module_id_t);

    // fatal_handler_t fatal = fatal_handler;
    // error_handler_t error = error_handler;
    // warning_handler_t warning = warning_handler;

    void operator()(const SourceLocation& location, Fatal, error_code_t code, module_id_t module)
    {
        (void)location;
        (void)code;
        (void)module;
        ++count;
        std::cout << "NumErrors " << count << std::endl;
    }

    void operator()(const SourceLocation& location, Error, error_code_t code, module_id_t module)
    {
        (void)location;
        (void)code;
        (void)module;
        ++count;
        std::cout << "NumErrors " << count << std::endl;
    }

    void operator()(const SourceLocation& location, Warning, error_code_t code, module_id_t module)
    {
        (void)location;
        (void)code;
        (void)module;
        ++count;
        std::cout << "NumErrors " << count << std::endl;
    }

    void preterminate()
    {
        std::cout << "COUNTING HANDLER WILL TERMINATE!" << std::endl;
    }

  private:
    int count{0};
};

CountingHandler& countingHandler()
{
    static CountingHandler h;
    return h;
}

// provides the currently installed unique handler, initially default
template <typename Interface, typename Default>
class UniqueHandler
{
  public:
    static Interface& get()
    {
        instance(); // sets the default handler once it is called for the first time
                    // TODO: how costly is this if it is discarded in subsequent calls?
        return *s_handler;
    }

    // we always require a valid handler to be set
    // enforce contract of valid handler
    static Interface* set(Interface& handler)
    {
        auto& h = instance();
        std::lock_guard<std::mutex> lock(h.m_mutex);
        if (h.m_isFinal)
        {
            std::terminate(); // we cannot set it after it is finalized (TODO: decide on reaction)
            return nullptr;
        }

        return s_handler.exchange(&handler);
    }

    // set back to default
    static Interface* reset()
    {
        auto& h = instance();
        std::lock_guard<std::mutex> lock(h.m_mutex);
        if (h.m_isFinal)
        {
            std::terminate(); // we cannot set it after it is finalized (TODO: decide on reaction)
            return nullptr;
        }

        return s_handler.exchange(&h.m_default);
    }

    static void finalize()
    {
        auto& h = instance();
        std::lock_guard<std::mutex> lock(h.m_mutex);
        h.m_isFinal.store(true);
    }

  private:
    // not a member to avoid indirection - we may have inidrection ininstance anyway (after the first construction?)
    static std::atomic<Interface*> s_handler;

    // avoid lots of static variables, these are not accessed often (set and finalize are rare)
    std::atomic_bool m_isFinal{false};
    std::mutex m_mutex;
    DefaultHandler m_default;

    UniqueHandler()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        s_handler = &m_default;
    }

    // there is only ever one of them but it cannot be accessed
    static UniqueHandler& instance()
    {
        static UniqueHandler h;
        return h;
    }
};

template <typename I, typename D>
std::atomic<I*> UniqueHandler<I, D>::s_handler;

using Handler = UniqueHandler<HandlerInterface, DefaultHandler>;

// report is called by proxy (template/inline call there will be optimized and not exist as a function call)
// 1 indirection s_handler
// 1 indirection vtable
// 1 indirection to call the desired function from vtable
//
// we may be able to get rid of 1 indirection (how?)
// if the dynamics are not required we save a lot of indirections

template <class Level, class Error>
void report(const SourceLocation& location, Level level, const Error& error)
{
    Handler::get()(location, level, error.code(), error.module());
}

template <class Level>
void report(ErrorStream& stream, const SourceLocation& location, Level level, error_code_t code, module_id_t module)
{
    Handler::get()(location, level, code, module);
}

// platform specific termination
void preterminate()
{
    Handler::get().preterminate();
}
} // namespace eh
