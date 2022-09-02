#pragma once

#ifdef TEST_PLATFORM

#include "test.hpp"

#include "iceoryx_hoofs/error_handling_2/platform/test_platform/test_handler.hpp"
#include "iceoryx_hoofs/error_handling_2/runtime_error.hpp"

// initialize handlers on first use
eh::TestHandler& testHandler()
{
    static eh::TestHandler h;
    return h;
}

template <typename Code, typename Level>
eh::RuntimeError toError(Code code, Level level)
{
    return eh::RuntimeError::from(eh::createError(code), level);
}

uint32_t countError(const eh::RuntimeError& error)
{
    return testHandler().errors().count(error);
}

bool terminationRequested()
{
    return testHandler().terminationRequested();
}

template <typename Code, typename Level>
bool expectError(Code code, Level level, uint32_t count = 1)
{
    auto error = toError(code, level);
    bool ret = !terminationRequested() && countError(error) == count;
    testHandler().reset();
    return ret;
}

template <typename Code>
bool expectFatalError(Code code, uint32_t count = 1)
{
    auto error = toError(code, eh::FATAL);
    bool ret = terminationRequested() && countError(error) == count;
    testHandler().reset();
    return ret;
}

// debatable, in general we should not expect errors by default
bool expectNoError()
{
    bool ret = testHandler().errors().count() == 0;
    testHandler().reset();
    return ret;
}

// macros to get line numbers
// can only deal with one error (count = 1, which could be extended)
#define EXPECT_ERROR(code, level) EXPECT_TRUE(expectError(code, level))
#define EXPECT_FATAL_ERROR(code) EXPECT_TRUE(expectFatalError(code))
#define EXPECT_NO_ERROR() EXPECT_TRUE(expectNoError())

#endif