#pragma once

#ifndef IOX_HOOFS_TEST_ERROR_HANDLER_HPP
#define IOX_HOOFS_TEST_ERROR_HANDLER_HPP

#include "iceoryx_hoofs/design_pattern/polymorphic_handler.hpp"
#include "iceoryx_hoofs/design_pattern/static_lifetime_guard.hpp"
#include "iceoryx_hoofs/error_reporting/error.hpp"
#include "iceoryx_hoofs/error_reporting/error_logging.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

#include <algorithm>
#include <atomic>
#include <vector>

// we can use this for test code
#include <mutex>

namespace iox
{
namespace err
{

/// @todo maybe another detail namespace

struct HandlerInterface
{
    virtual ~HandlerInterface() = default;

    virtual void panic() = 0;

    // alternatively we have to use a generic base error
    // it could be based on "any", but this would be more expensive
    virtual void report(const SourceLocation& location, error_code_t code) = 0;

    // virtual void report(const SourceLocation& location, error_code_t code, const char* message) = 0;
};

class DefaultHandler : public HandlerInterface
{
  public:
    void report(const SourceLocation&, error_code_t) override
    {
    }

    void panic() override
    {
    }
};

class TestHandler : public HandlerInterface
{
  public:
    void report(const SourceLocation&, error_code_t code) override
    {
        std::lock_guard<std::mutex> g(m_mutex);
        m_errors.push_back(code);
    }

    void panic() override
    {
        m_panicked = true;
    }

    bool hasPanicked()
    {
        return m_panicked;
    }

    void reset()
    {
        m_panicked = false;
        std::lock_guard<std::mutex> g(m_mutex);
        m_errors.clear();
    }

    bool hasError() const
    {
        return !m_errors.empty();
    }

    bool hasError(error_code_t code) const
    {
        std::lock_guard<std::mutex> g(m_mutex);
        auto iter = std::find(m_errors.begin(), m_errors.end(), code);
        return iter != m_errors.end();
    }

  private:
    mutable std::mutex m_mutex;
    std::atomic<bool> m_panicked{false};
    std::vector<error_code_t> m_errors;
};

template <typename T>
using Guard = iox::design_pattern::StaticLifetimeGuard<T>;
using ErrorHandler = iox::design_pattern::PolymorphicHandler<HandlerInterface, DefaultHandler>;

// alias for usability, hides the guard
using TestErrorHandler = Guard<TestHandler>;
using DefaultErrorHandler = Guard<DefaultHandler>;

} // namespace err
} // namespace iox

#endif
