#pragma once

#include "default_handler.hpp"
#include "polymorphic_handler.hpp"

namespace eh
{
using ErrorHandler = PolymorphicHandler<HandlerInterface, DefaultHandler>;

// report is called by proxy (template/inline call there will be optimized and not exist as a function call)
// 1 indirection get
// 1 indirection vtable
// 1 indirection to call the desired function from vtable
//
// we can get rid of 1 indirection (with direct usage of function pointers without vtable)
// if the dynamics are not required we save all indirections

template <class Level, class Error>
void report(const SourceLocation& location, Level level, const Error& error)
{
    // NB: this ErrorHandler interface loses information about the concrete
    // error class
    // This is not strictly necessary if the Error classes are well-defined
    // and e.g. follow an ineritance hierarchy (possible but intentionally not required).

    std::cout << "REPORT " << std::endl;
    ErrorHandler::get()(location, level, error.code(), error.module());
}

// platform specific termination
inline void preterminate()
{
    ErrorHandler::get().preterminate();
}

} // namespace eh
