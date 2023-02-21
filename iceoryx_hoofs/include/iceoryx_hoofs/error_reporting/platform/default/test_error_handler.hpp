#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_TEST_ERROR_HANDLER_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_TEST_ERROR_HANDLER_HPP

#include "iceoryx_hoofs/error_reporting/error.hpp"
#include "iceoryx_hoofs/error_reporting/error_logging.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"
#include "iceoryx_hoofs/error_reporting/platform/default/error_handler_interface.hpp"

#include <algorithm>
#include <atomic>
#include <vector>

// we can use this for test code
#include <mutex>

// NOLINTNEXTLINE(hicpp-deprecated-headers) required to work on some platforms
#include <setjmp.h>

namespace iox
{
namespace err
{

/// @brief Defines the test reaction of dynamic error handling.
class TestHandler : public ErrorHandlerInterface
{
  public:
    /// @brief Defines the reaction on panic.
    void panic() override;

    /// @brief Defines the reaction on error.
    /// @param location the location of the error
    /// @param code the code of the error
    void report(const SourceLocation&, ErrorCode code) override;

    /// @brief Indicates whether there was a panic call previously.
    /// @return true if there was a panic call, false otherwise
    bool hasPanicked();

    /// @brief Reset panic state and clears all errors that occurred previously.
    void reset();

    /// @brief Indicates whether any error occurred previously.
    bool hasError() const;

    /// @todo module id must be propagated here as well
    /// @brief Indicates whether as specific error occurred previously.
    bool hasError(ErrorCode code) const;

    /// @brief Prepare a jump and return jump buffer
    /// @return true if the jump destination was set, false otherwise (jump occurred)
    /// @note setjmp cannot be called in such a helper function (must still be on stack once
    /// longjmp is called)
    jmp_buf& prepareJump();

    /// @brief Returns the value that is set by longjmp in case of a jump.
    /// @return the jump indicator value
    static int jumpIndicator();

  private:
    static constexpr int JUMPED{1};

    mutable std::mutex m_mutex;
    std::atomic<bool> m_panicked{false};
    std::vector<ErrorCode> m_errors;

    jmp_buf m_jumpBuffer;
    std::atomic<bool> m_jump{false};

    void jump();
};

} // namespace err
} // namespace iox

#endif
