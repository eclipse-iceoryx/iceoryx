#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_HANDLER_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_HANDLER_HPP

#include "iceoryx_hoofs/design_pattern/polymorphic_handler.hpp"
#include "iceoryx_hoofs/design_pattern/static_lifetime_guard.hpp"

#include "iceoryx_hoofs/error_reporting/platform/default/default_error_handler.hpp"
#include "iceoryx_hoofs/error_reporting/platform/default/error_handler_interface.hpp"
#include "iceoryx_hoofs/error_reporting/platform/default/test_error_handler.hpp"

namespace iox
{
namespace err
{

// this is to be used later
using ErrorHandler = iox::design_pattern::PolymorphicHandler<ErrorHandlerInterface, DefaultHandler>;

// alias for usability, hides the guard
using TestErrorHandler = iox::design_pattern::StaticLifetimeGuard<TestHandler>;
using DefaultErrorHandler = iox::design_pattern::StaticLifetimeGuard<DefaultHandler>;

} // namespace err
} // namespace iox

#endif
