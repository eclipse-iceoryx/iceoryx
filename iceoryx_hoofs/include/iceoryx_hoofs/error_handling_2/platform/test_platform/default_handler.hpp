#pragma once

#include "handler_interface.hpp"

#include <iostream>

namespace eh
{
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
    }
};
} // namespace eh
