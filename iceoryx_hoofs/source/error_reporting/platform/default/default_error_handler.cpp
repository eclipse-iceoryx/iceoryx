#include "iceoryx_hoofs/error_reporting/platform/default/default_error_handler.hpp"

namespace iox
{
namespace err
{

void DefaultHandler::reportError(ErrorDescriptor)
{
}

void DefaultHandler::reportViolation(ErrorDescriptor)
{
}

void DefaultHandler::panic()
{
}

} // namespace err
} // namespace iox