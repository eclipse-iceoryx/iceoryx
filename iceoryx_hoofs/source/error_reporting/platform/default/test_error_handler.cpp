#include "iceoryx_hoofs/error_reporting/platform/default/test_error_handler.hpp"

namespace iox
{
namespace err
{

void TestHandler::report(const SourceLocation&, ErrorCode code)
{
    std::lock_guard<std::mutex> g(m_mutex);
    m_errors.push_back(code);
}

void TestHandler::panic()
{
    m_panicked = true;
    jump();
}

bool TestHandler::hasPanicked()
{
    return m_panicked;
}

void TestHandler::reset()
{
    m_panicked = false;
    std::lock_guard<std::mutex> g(m_mutex);
    m_errors.clear();
}

bool TestHandler::hasError() const
{
    return !m_errors.empty();
}

bool TestHandler::hasError(ErrorCode code) const
{
    /// @todo use module id as well
    std::lock_guard<std::mutex> g(m_mutex);
    auto iter = std::find(m_errors.begin(), m_errors.end(), code);
    return iter != m_errors.end();
}

jmp_buf& TestHandler::prepareJump()
{
    m_jump = true;
    return m_jumpBuffer;
}

void TestHandler::jump()
{
    if (m_jump)
    {
        // NOLINTNEXTLINE(cert-err52-cpp) exception handling is not used by design
        longjmp(&m_jumpBuffer[0], jumpIndicator());
    }
}

int TestHandler::jumpIndicator()
{
    return JUMPED;
}

} // namespace err
} // namespace iox