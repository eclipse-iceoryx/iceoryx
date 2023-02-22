#include "iceoryx_hoofs/testing/error_reporting/test_error_handler.hpp"
#include <csetjmp>

namespace iox
{
namespace testing
{

using namespace iox::err;

TestHandler::TestHandler()
    : m_jump(&m_jumpBuffer)
{
}

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
    std::lock_guard<std::mutex> g(m_mutex);
    m_panicked = false;
    m_errors.clear();
    m_jump.store(&m_jumpBuffer);
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

jmp_buf* TestHandler::prepareJump()
{
    // winner can prepare the jump
    return m_jump.exchange(nullptr);
}

void TestHandler::jump()
{
    jmp_buf* exp = nullptr;
    // if it is a nullptr, somebody (and only one) has prepared jump
    // it will be reset on first jump, so there cannot be concurrent jumps
    // essentially the first panic call wins, resets and and jumps
    if (m_jump.compare_exchange_strong(exp, &m_jumpBuffer))
    {
        // NOLINTNEXTLINE(cert-err52-cpp) exception handling is not used by design
        longjmp(&m_jumpBuffer[0], jumpIndicator());
    }
}

int TestHandler::jumpIndicator()
{
    return JUMPED;
}

} // namespace testing
} // namespace iox