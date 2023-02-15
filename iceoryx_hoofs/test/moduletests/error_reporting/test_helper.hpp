#pragma once

#include "iceoryx_hoofs/cxx/function_ref.hpp"
#include "iceoryx_hoofs/error_reporting/platform/default/error_handler.hpp"

#include <thread>
#include <utility>

#define ASSERT_NO_PANIC()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_FALSE(iox::err::TestErrorHandler::instance().hasPanicked());                                            \
    } while (false)

#define ASSERT_PANIC()                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_TRUE(iox::err::TestErrorHandler::instance().hasPanicked());                                             \
    } while (false)

#define ASSERT_ERROR(code)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        auto e = error_code_t(code);                                                                                   \
        ASSERT_TRUE(iox::err::TestErrorHandler::instance().hasError(e));                                               \
    } while (false)

namespace iox
{
namespace testing
{

void runInTestThread(cxx::function_ref<void(void)> testFunction)
{
    auto f = [&]() {
        if (iox::err::TestErrorHandler::instance().setJump())
        {
            testFunction();
        }
    };

    std::thread t(f);
    if (t.joinable())
    {
        t.join();
    }
}

/// @todo this should work but somehow does not, investigate
/// this would increase generality/elegance mainly
// template <typename Function, typename... Args>
// void runInTestThread(Function&& testFunction, Args&&... args)
// {
//     auto f = [&]() {
//         if (iox::err::TestErrorHandler::instance().setJump())
//         {
//             testFunction(std::forward<Args>(args)...);
//         }
//     };

//     std::thread t(f);
//     t.join();
// }

} // namespace testing
} // namespace iox