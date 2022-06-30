#pragma once

#include "error_storage.hpp"
#include "handler_interface.hpp"

#include <atomic>
#include <iostream>

namespace eh
{
struct TestHandler : public HandlerInterface
{
    void operator()(const SourceLocation& location, Fatal, error_code_t code, module_id_t module)
    {
        (void)location;
        storeError(code, module);
    }

    void operator()(const SourceLocation& location, Error, error_code_t code, module_id_t module)
    {
        (void)location;
        storeError(code, module);
    }

    void operator()(const SourceLocation& location, Warning, error_code_t code, module_id_t module)
    {
        (void)location;
        storeError(code, module);
    }

    void preterminate()
    {
        std::cout << "COUNTING HANDLER WILL TERMINATE!" << std::endl;
    }

    void reset()
    {
        m_count = 0;
        m_errors.reset();
    }

    const ErrorStorage& errors()
    {
        return m_errors;
    }

  private:
    std::atomic<int> m_count{0};
    ErrorStorage m_errors;

    // we could distinguish between error levels as well etc.
    void storeError(error_code_t code, module_id_t module)
    {
        std::cout << "num errors " << ++m_count << std::endl;
        m_errors.add(RuntimeError(module, code));
    }
};

struct ThrowHandler : public HandlerInterface
{
    void operator()(const SourceLocation& location, Fatal, error_code_t code, module_id_t module)
    {
        (void)location;
        // note that throwing earlier in the static part is more general as we
        // have full type information about the error at this point
        // (here it is erased but still sufficient)
        throw(RuntimeError(module, code));
    }

    void operator()(const SourceLocation& location, Error, error_code_t code, module_id_t module)
    {
        (void)location;
        throw(RuntimeError(module, code));
    }

    void operator()(const SourceLocation& location, Warning, error_code_t code, module_id_t module)
    {
        (void)location;
        throw(RuntimeError(module, code));
    }

    void preterminate()
    {
        std::cout << "THROWING HANDLER WILL TERMINATE!" << std::endl;
    }
};

} // namespace eh