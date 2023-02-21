#include "iceoryx_hoofs/error_reporting/platform/default/default_error_handler.hpp"

namespace iox
{
namespace err
{

void DefaultHandler::report(const SourceLocation&, ErrorCode)
{
}

void DefaultHandler::panic()
{
}

} // namespace err
} // namespace iox