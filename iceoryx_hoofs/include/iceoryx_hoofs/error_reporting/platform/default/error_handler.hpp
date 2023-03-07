#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_HANDLER_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_HANDLER_HPP

#include "iox/polymorphic_handler.hpp"
#include "iox/static_lifetime_guard.hpp"

#include "iceoryx_hoofs/error_reporting/platform/default/default_error_handler.hpp"
#include "iceoryx_hoofs/error_reporting/platform/default/error_handler_interface.hpp"

namespace iox
{
namespace err
{

// this is to be used later
using ErrorHandler = iox::PolymorphicHandler<ErrorHandlerInterface, DefaultHandler>;

using DefaultErrorHandler = iox::StaticLifetimeGuard<DefaultHandler>;

} // namespace err
} // namespace iox

#endif
