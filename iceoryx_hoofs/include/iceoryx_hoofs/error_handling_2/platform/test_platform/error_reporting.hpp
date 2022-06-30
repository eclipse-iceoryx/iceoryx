#pragma once

#include "default_handler.hpp"
#include "handler_interface.hpp"

namespace eh
{
using ErrorHandler = UniqueHandler<HandlerInterface, DefaultHandler>;

// report is called by proxy (template/inline call there will be optimized and not exist as a function call)
// 1 indirection s_handler
// 1 indirection vtable
// 1 indirection to call the desired function from vtable
//
// we may be able to get rid of 1 indirection (with direct usage of function pointers without vtable)
// if the dynamics are not required we save a lot of indirections

template <class Level, class Error>
void report(const SourceLocation& location, Level level, const Error& error)
{
    ErrorHandler::get()(location, level, error.code(), error.module());
}

// platform specific termination
void preterminate()
{
    ErrorHandler::get().preterminate();
}

} // namespace eh