#pragma once

#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"
#include "iceoryx_hoofs/error_handling_2/platform/error_levels.hpp"

#include <atomic>
#include <mutex>

namespace eh
{
// can be defined by the platform and will/should depend on the error levels of the platform
// the cleint is otherwise free to define a handler interface as she sees fit (to be useful it
// should generally at least include a reaction to the different error levels)

struct HandlerInterface
{
    virtual void operator()(const SourceLocation& location, Fatal, error_code_t code, module_id_t module) = 0;
    virtual void operator()(const SourceLocation& location, Error, error_code_t code, module_id_t module) = 0;
    virtual void operator()(const SourceLocation& location, Warning, error_code_t code, module_id_t module) = 0;

    virtual void preterminate() = 0;
};

// infrastructure to have exactly one handler installed

// provides the currently installed unique handler, initially default
template <typename Interface, typename DefaultHandler>
class UniqueHandler
{
  public:
    static Interface& get()
    {
        return *s_handler.load(std::memory_order_relaxed);
    }

    // we always require a valid handler to be set
    // enforce contract of valid handler
    static Interface* set(Interface& handler)
    {
        auto& h = instance();
        std::lock_guard<std::mutex> lock(h.m_mutex);
        if (h.m_isFinal)
        {
            std::cerr << "SETTING THE ERROR HANDLER AFTER FIANALIZE IS NOT ALLOWED!" << std::endl;
            std::terminate();
            return nullptr;
        }

        return s_handler.exchange(&handler, std::memory_order_relaxed);
    }

    static Interface* reset()
    {
        return set(s_default);
    }

    static void finalize()
    {
        auto& h = instance();
        std::lock_guard<std::mutex> lock(h.m_mutex);
        h.m_isFinal.store(true);
    }

  private:
    static DefaultHandler s_default; // ensure there is always at least a default handler
    // not a member to avoid indirection - we may have inidrection ininstance anyway
    static std::atomic<Interface*> s_handler;

    // avoid lots of static variables, these are not accessed often (set and finalize are rare)
    std::atomic_bool m_isFinal{false};
    std::mutex m_mutex; // required for sync only

    // there is only ever one of them but it cannot be accessed
    static UniqueHandler& instance()
    {
        static UniqueHandler h;
        return h;
    }
};

// these are initialized before any static method (set, get etc.) is called(!)
template <typename I, typename D>
D UniqueHandler<I, D>::s_default{};

template <typename I, typename D>
std::atomic<I*> UniqueHandler<I, D>::s_handler{&s_default};

} // namespace eh