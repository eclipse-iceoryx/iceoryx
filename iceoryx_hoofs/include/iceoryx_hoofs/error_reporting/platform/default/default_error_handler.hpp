#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_ERROR_HANDLER_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_ERROR_HANDLER_HPP

#include "iceoryx_hoofs/error_reporting/platform/default/error_handler_interface.hpp"

namespace iox
{
namespace err
{

/// @brief Defines the default reaction of dynamic error handling.
class DefaultHandler : public ErrorHandlerInterface
{
  public:
    /// @brief Defines the reaction on panic.
    void panic() override;

    /// @brief Defines the reaction on error.
    /// @param location the location of the error
    /// @param code the code of the error
    void report(const SourceLocation&, ErrorCode) override;
};

} // namespace err
} // namespace iox

#endif
