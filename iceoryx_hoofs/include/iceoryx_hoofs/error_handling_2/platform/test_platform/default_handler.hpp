#pragma once

#include "handler_interface.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"

namespace eh
{
struct DefaultHandler : public HandlerInterface
{
    DefaultHandler() = default;
    ~DefaultHandler() override = default;

    void operator()(const SourceLocation& location, Fatal, error_code_t code, module_id_t module) override
    {
        (void)location;
        (void)code;
        (void)module;
    }

    void operator()(const SourceLocation& location, Error, error_code_t code, module_id_t module) override
    {
        (void)location;
        (void)code;
        (void)module;
    }

    void operator()(const SourceLocation& location, Warning, error_code_t code, module_id_t module) override
    {
        (void)location;
        (void)code;
        (void)module;
    }

    inline void preterminate() override
    {
    }
};
} // namespace eh
