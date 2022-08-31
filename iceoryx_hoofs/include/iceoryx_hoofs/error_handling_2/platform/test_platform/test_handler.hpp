#pragma once

#include "error_storage.hpp"
#include "handler_interface.hpp"
#include "iceoryx_hoofs/error_handling_2/error_level.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"

#include <atomic>
#include <iostream>

namespace eh
{
struct TestHandler : public HandlerInterface
{
    void operator()(const SourceLocation& location, Fatal, error_code_t code, module_id_t module) override
    {
        (void)location;
        storeError(code, module, Fatal::value);
    }

    void operator()(const SourceLocation& location, Error, error_code_t code, module_id_t module) override
    {
        (void)location;
        storeError(code, module, Error::value);
    }

    void operator()(const SourceLocation& location, Warning, error_code_t code, module_id_t module) override
    {
        (void)location;
        storeError(code, module, Warning::value);
    }

    void preterminate() override
    {
        m_terminate = true;
    }

    void reset()
    {
        m_count = 0;
        m_errors.reset();
        m_terminate = false;
    }

    const ErrorStorage& errors()
    {
        return m_errors;
    }

    bool terminationRequested()
    {
        return m_terminate;
    }

  private:
    std::atomic<int> m_count{0};
    ErrorStorage m_errors;
    bool m_terminate{false};

    // we could distinguish between error levels as well etc.
    void storeError(error_code_t code, module_id_t module, error_level_t level)
    {
        m_errors.add(RuntimeError(module, code, level));
    }
};

struct ThrowHandler : public HandlerInterface
{
    void operator()(const SourceLocation& location, Fatal, error_code_t code, module_id_t module) override
    {
        (void)location;
        // note that throwing earlier in the static part is more general as we
        // have full type information about the error at this point
        // (here it is erased but still sufficient)
        throw(RuntimeError(module, code, Fatal::value));
    }

    void operator()(const SourceLocation& location, Error, error_code_t code, module_id_t module) override
    {
        (void)location;
        throw(RuntimeError(module, code, Error::value));
    }

    void operator()(const SourceLocation& location, Warning, error_code_t code, module_id_t module) override
    {
        (void)location;
        throw(RuntimeError(module, code, Warning::value));
    }

    void preterminate() override
    {
    }
};

} // namespace eh