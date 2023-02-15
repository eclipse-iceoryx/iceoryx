#include "iceoryx_hoofs/error_reporting/platform/default/default_error_handler.hpp"

namespace iox
{
namespace err
{

void DefaultHandler::report(const SourceLocation&, error_code_t)
{
}

void DefaultHandler::panic()
{
}

} // namespace err
} // namespace iox