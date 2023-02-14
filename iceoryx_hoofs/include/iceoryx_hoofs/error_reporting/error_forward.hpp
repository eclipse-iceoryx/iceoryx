#pragma once

#include "iceoryx_hoofs/error_reporting/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/error_logging.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

#include "platform/error_reporting.hpp"

#include <utility>

namespace iox
{
namespace err
{

template <typename Error, typename Kind>
/// @todo make noreturn once combined with fatal failure testing (longjmp)
///[[noreturn]]
void forwardFatalError(const SourceLocation& location, Error&& error, Kind&& kind)
{
    report(location, kind, error);
    panic();

    // acivate later to satisfy the noreturn guarantee
    // abort();
}

template <typename Error, typename Kind>
void forwardNonFatalError(const SourceLocation& location, Error&& error, Kind&& kind)
{
    report(location, kind, error);
}

template <typename Error, typename Kind>
void forwardError(const SourceLocation& location, Error&& error, Kind&& kind)
{
    // forwarding selection happens at compile time
    // important: the fatal branch is visibly no-return for the compiler here
    if (requiresHandling(kind))
    {
        if (isFatal(kind))
        {
            forwardFatalError(location, std::forward<Error>(error), std::forward<Kind>(kind));
        }
        else
        {
            forwardNonFatalError(location, std::forward<Error>(error), std::forward<Kind>(kind));
        }
    }
}

// version with message, separate overload is the efficient solution

template <typename Error, typename Kind, typename Message>
/// @todo make noreturn once combined with fatal failure testing (longjmp)
///[[noreturn]]
void forwardFatalError(const SourceLocation& location, Error&& error, Kind&& kind, Message&& msg)
{
    report(location, kind, error, msg);
    panic();

    // acivate later to satisfy the noreturn guarantee
    // abort();
}

template <typename Error, typename Kind, typename Message>
void forwardNonFatalError(const SourceLocation& location, Error&& error, Kind&& kind, Message&& msg)
{
    report(location, kind, error, msg);
}

template <typename Error, typename Kind, typename Message>
void forwardError(const SourceLocation& location, Error&& error, Kind&& kind, Message&& msg)
{
    // forwarding selection happens at compile time
    if (requiresHandling(kind))
    {
        if (isFatal(kind))
        {
            forwardFatalError(
                location, std::forward<Error>(error), std::forward<Kind>(kind), std::forward<Message>(msg));
        }
        else
        {
            forwardNonFatalError(
                location, std::forward<Error>(error), std::forward<Kind>(kind), std::forward<Message>(msg));
        }
    }
}

// this is used to avoid warnings if the expression is not used other than in the assertion
// it is compiled out by optimizing compilers
template <typename Lambda>
void discard(Lambda&& lambda)
{
    // enforce that it is not evaluated
    if (false)
    {
        lambda();
    }
}

} // namespace err
} // namespace iox
