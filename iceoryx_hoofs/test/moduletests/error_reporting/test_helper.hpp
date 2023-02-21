#ifndef IOX_HOOFS_MODULETESTS_ERROR_REPORTING_TEST_HELPER_HPP
#define IOX_HOOFS_MODULETESTS_ERROR_REPORTING_TEST_HELPER_HPP

#include "iceoryx_hoofs/error_reporting/platform/default/error_handler.hpp"

#include <thread>
#include <utility>

// NOLINTNEXTLINE(hicpp-deprecated-headers) required to work on some platforms
#include <setjmp.h>

namespace iox
{
namespace testing
{

/// @brief indicates whether the test error handler invoked panic
inline bool hasPanicked()
{
    return iox::err::TestErrorHandler::instance().hasPanicked();
}

/// @brief indicates whether the test error handler registered a specific error
template <typename Code>
inline bool hasError(Code&& code)
{
    auto e = iox::err::toError(std::forward<Code>(code));
    return iox::err::TestErrorHandler::instance().hasError(e.code());
}

/// @brief runs testFunction in a testContext that can detect fatal failures;
/// runs in the same thread
/// @note uses a longjump
template <typename Function, typename... Args>
inline void testContext(Function&& testFunction, Args&&... args)
{
    auto& buf = iox::err::TestErrorHandler::instance().prepareJump();

    // setjmp must be called in a stackframe that still exists when longjmp is called
    // Therefore there cannot be a convenient abstraction that does not also
    // know the test function that is being called.
    // NOLINTNEXTLINE
    if (setjmp(&buf[0]) != iox::err::TestErrorHandler::instance().jumpIndicator())
    {
        testFunction(std::forward<Args>(args)...);
    }
}

/// @brief runs testFunction in a testContext that can detect fatal failures;
/// runs in a separate thread
/// @note uses a longjump inside the thread it runs the function in
template <typename Function, typename... Args>
inline void runInTestThread(Function&& testFunction, Args&&... args)
{
    // needed to infer the testContext arguments
    auto f = [&]() { testContext(std::forward<Function>(testFunction), std::forward<Args>(args)...); };

    std::thread t(f);
    if (t.joinable())
    {
        t.join();
    }
}

} // namespace testing
} // namespace iox

#define ASSERT_NO_PANIC()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_FALSE(iox::testing::hasPanicked());                                                                     \
    } while (false)

#define ASSERT_PANIC()                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_TRUE(iox::testing::hasPanicked());                                                                      \
    } while (false)

#define ASSERT_ERROR(code)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_TRUE(iox::testing::hasError(code));                                                                     \
    } while (false)

#define EXPECT_NO_PANIC()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        EXPECT_FALSE(iox::testing::hasPanicked());                                                                     \
    } while (false)

#define EXPECT_PANIC()                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        EXPECT_TRUE(iox::testing::hasPanicked());                                                                      \
    } while (false)

#define EXPECT_ERROR(code)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        EXPECT_TRUE(iox::testing::hasError(code));                                                                     \
    } while (false)
#endif
