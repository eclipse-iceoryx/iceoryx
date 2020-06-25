#include "timing_test.hpp"

namespace iox
{
namespace testutils
{
bool performingTimingTest(const std::function<void()>& testCallback,
                          const uint64_t repetitions,
                          std::atomic_bool& testResult) noexcept
{
    for (uint64_t i = 0u; i < repetitions; ++i)
    {
        // new test run therefore we have to reset the testResult
        testResult.store(true);
        // testResult will be set to false if a test failes
        testCallback();

        if (testResult.load())
        {
            return true;
        }
    }
    return false;
}

std::string verifyTimingTestResult(const char* file,
                                   const int line,
                                   const char* valueStr,
                                   const bool value,
                                   const bool expected,
                                   std::atomic_bool& result) noexcept
{
    std::string errorMessage;
    if (value != expected)
    {
        errorMessage += "Timing Test failure in:\n";
        errorMessage += std::string(file) + ":" + std::to_string(line) + "\n";
        errorMessage += "Value of: " + std::string(valueStr) + " should be true\n";
        result.store(false);
    }
    return errorMessage;
}
} // namespace testutils
} // namespace iox
