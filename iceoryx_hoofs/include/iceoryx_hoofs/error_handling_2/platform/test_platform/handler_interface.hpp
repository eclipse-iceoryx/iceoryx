#pragma once

#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"
#include "iceoryx_hoofs/error_handling_2/platform/error_levels.hpp"

#include <atomic>

namespace eh
{
// can be defined by the platform and will/should depend on the error levels of the platform
// the cleint is otherwise free to define a handler interface as she sees fit (to be useful it
// should generally at least include a reaction to the different error levels)

struct HandlerInterface
{
    virtual ~HandlerInterface() = default;

    virtual void operator()(const SourceLocation& location, Fatal, error_code_t code, module_id_t module) = 0;
    virtual void operator()(const SourceLocation& location, Error, error_code_t code, module_id_t module) = 0;
    virtual void operator()(const SourceLocation& location, Warning, error_code_t code, module_id_t module) = 0;

    virtual void preterminate() = 0;

    bool isActive()
    {
        return m_active;
    }

    void activate()
    {
        m_active = true;
    }

    void deactivate()
    {
        m_active = false;
    }

  private:
    std::atomic_bool m_active{true};
};

} // namespace eh