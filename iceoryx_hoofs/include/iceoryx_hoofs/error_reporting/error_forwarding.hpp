#ifndef IOX_HOOFS_ERROR_REPORTING_ERROR_FORWARDING_HPP
#define IOX_HOOFS_ERROR_REPORTING_ERROR_FORWARDING_HPP

#include "iceoryx_hoofs/error_reporting/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/error_logging.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

#include "iceoryx_hoofs/error_reporting/platform/error_reporting.hpp"

#include <utility>

namespace iox
{
namespace err
{
/// @brief Forwards a fatal error and does not return.
/// @param location the location of the error
/// @param error the error
/// @param kind the kind of error (category)
template <typename Error, typename Kind>
[[noreturn]] void forwardFatalError(const SourceLocation& location, Error&& error, Kind&& kind)
{
    report(location, kind, error);
    panic(location);
    abort();
}

/// @brief Forwards a non-fatal error.
/// @param location the location of the error
/// @param error the error
/// @param kind the kind of error (category)
template <typename Error, typename Kind>
void forwardNonFatalError(const SourceLocation& location, Error&& error, Kind&& kind)
{
    report(location, kind, error);
}

/// @brief Forwards a fatal or non-fatal error.
/// @param location the location of the error
/// @param error the error
/// @param kind the kind of error (category)
template <typename Error, typename Kind>
void forwardError(const SourceLocation& location, Error&& error, Kind&& kind)
{
    // forwarding selection happens at compile time
    // important: the fatal branch is visibly no-return for the compiler here
    if (isFatal(kind))
    {
        forwardFatalError(location, std::forward<Error>(error), std::forward<Kind>(kind));
    }
    else
    {
        forwardNonFatalError(location, std::forward<Error>(error), std::forward<Kind>(kind));
    }
}

/// @brief Forwards a fatal error and a message and does not return.
/// @param location the location of the error
/// @param error the error
/// @param kind the kind of error (category)
/// @param msg the message to be forwarded
template <typename Error, typename Kind, typename Message>
[[noreturn]] void forwardFatalError(const SourceLocation& location, Error&& error, Kind&& kind, Message&& msg)
{
    report(location, kind, error, msg);
    panic(location);
    abort();
}

/// @brief Forwards a non-fatal error and a message.
/// @param location the location of the error
/// @param error the error
/// @param kind the kind of error (category)
/// @param msg the message to be forwarded
template <typename Error, typename Kind, typename Message>
void forwardNonFatalError(const SourceLocation& location, Error&& error, Kind&& kind, Message&& msg)
{
    report(location, kind, error, msg);
}

/// @brief Forwards a fatal or non-fatal error and a message.
/// @param location the location of the error
/// @param error the error
/// @param kind the kind of error (category)
/// @param msg the message to be forwarded
template <typename Error, typename Kind, typename Message>
void forwardError(const SourceLocation& location, Error&& error, Kind&& kind, Message&& msg)
{
    // forwarding selection happens at compile time
    if (isFatal(kind))
    {
        forwardFatalError(location, std::forward<Error>(error), std::forward<Kind>(kind), std::forward<Message>(msg));
    }
    else
    {
        forwardNonFatalError(
            location, std::forward<Error>(error), std::forward<Kind>(kind), std::forward<Message>(msg));
    }
}

/// @brief Discards some generic values.
/// @note used to suppress unused variable warnings if certain checks are disabled,
/// the artificial use of value will be optimized away by the compiler.
template <typename... Args>
void discard(Args&&...)
{
}

} // namespace err
} // namespace iox

#endif
