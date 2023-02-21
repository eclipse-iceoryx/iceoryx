#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_HANDLER_INTERFACE_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_HANDLER_INTERFACE_HPP

#include "iceoryx_hoofs/error_reporting/error.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"
#include "iceoryx_hoofs/error_reporting/types.hpp"

namespace iox
{
namespace err
{
/// @brief Defines the dynamic error handling interface (i.e. changeable at runtime).
/// NOLINTJUSTIFICATION abstract interface
/// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class ErrorHandlerInterface
{
  public:
    virtual ~ErrorHandlerInterface() = default;

    /// @brief Defines the reaction on panic.
    virtual void panic() = 0;

    /// @brief Defines the reaction on error.
    /// @param location the location of the error
    /// @param code the code of the error
    /// @note some of the code that is always supposed to be exectuted is factored out at call-site
    virtual void report(const SourceLocation& location, ErrorCode code) = 0;
};

} // namespace err
} // namespace iox

#endif
