#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_HANDLER_INTERFACE_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_HANDLER_INTERFACE_HPP

#include "iceoryx_hoofs/error_reporting/error.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

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
    /// @note we distinguish the error by error_code_t, alternatively we would need an error
    /// class hierarchy which would be heavier and is not needed now (can be extended in the future)
    virtual void report(const SourceLocation& location, error_code_t code) = 0;
};

} // namespace err
} // namespace iox

#endif
