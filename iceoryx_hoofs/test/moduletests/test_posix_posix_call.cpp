// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "test.hpp"

using namespace ::testing;

namespace
{
int testFunction(int returnValue, int errnoValue)
{
    errno = errnoValue;
    return returnValue;
}

int32_t eintrRepetition = 0;
int testEintr()
{
    if (0 < --eintrRepetition)
    {
        errno = EINTR;
        return 1;
    }
    return 0;
}

int returnValueIsErrno(int returnValue)
{
    errno = 0;
    return returnValue;
}

class PosixCall_test : public Test
{
  public:
};
} // namespace

TEST_F(PosixCall_test, CallingFunctionWithSuccessReturnValue_GoodCase)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 1;
    constexpr int ERRNO_VALUE = 2;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(RETURN_VALUE)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithSuccessReturnValue_BadCase)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 3;
    constexpr int ERRNO_VALUE = 4;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(RETURN_VALUE + 1)
        .evaluate()
        .and_then([](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    // we expect an error message via stderr to the console, details are not
    // verified since it depends on the target and where the source code is
    // stored
    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithFailureReturnValue_GoodCase)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 5;
    constexpr int ERRNO_VALUE = 6;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .failureReturnValue(RETURN_VALUE + 1)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithFailureReturnValue_BadCase)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 7;
    constexpr int ERRNO_VALUE = 8;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .failureReturnValue(RETURN_VALUE)
        .evaluate()
        .and_then([](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    // we expect an error message via stderr to the console, details are not
    // verified since it depends on the target and where the source code is
    // stored
    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithSuccessReturnValueAndIgnoredErrno_GoodCase)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 9;
    constexpr int ERRNO_VALUE = 10;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(RETURN_VALUE + 1)
        .ignoreErrnos(ERRNO_VALUE)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithSuccessReturnValueAndIgnoredErrno_BadCase)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 11;
    constexpr int ERRNO_VALUE = 12;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(RETURN_VALUE + 1)
        .ignoreErrnos(ERRNO_VALUE + 1)
        .evaluate()
        .and_then([](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    // we expect an error message via stderr to the console, details are not
    // verified since it depends on the target and where the source code is
    // stored
    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithFailureReturnValueAndIgnoredErrno_GoodCase)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 13;
    constexpr int ERRNO_VALUE = 14;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .failureReturnValue(RETURN_VALUE)
        .ignoreErrnos(ERRNO_VALUE)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithFailureReturnValueAndIgnoredErrno_BadCase)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 15;
    constexpr int ERRNO_VALUE = 16;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .failureReturnValue(RETURN_VALUE)
        .ignoreErrnos(ERRNO_VALUE + 1)
        .evaluate()
        .and_then([](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    // we expect an error message via stderr to the console, details are not
    // verified since it depends on the target and where the source code is
    // stored
    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, IgnoringMultipleErrnosWorks)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 17;
    constexpr int ERRNO_VALUE = 18;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .ignoreErrnos(ERRNO_VALUE - 10, ERRNO_VALUE, ERRNO_VALUE + 17)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, IgnoringMultipleErrnosWhereOccurringErrnoIsNotListedFails)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 19;
    constexpr int ERRNO_VALUE = 20;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .ignoreErrnos(ERRNO_VALUE - 10, ERRNO_VALUE + 17, ERRNO_VALUE + 1337, ERRNO_VALUE - 2)
        .evaluate()
        .and_then([](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, IgnoringMultipleErrnosWhereOccurringErrnoIsFirstInListSucceeds)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 21;
    constexpr int ERRNO_VALUE = 22;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .ignoreErrnos(ERRNO_VALUE, ERRNO_VALUE - 91, ERRNO_VALUE + 137, ERRNO_VALUE + 17, ERRNO_VALUE - 29)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, IgnoringMultipleErrnosWhereOccurringErrnoIsLastInListSucceeds)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 23;
    constexpr int ERRNO_VALUE = 24;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .ignoreErrnos(ERRNO_VALUE - 918, ERRNO_VALUE + 8137, ERRNO_VALUE + 187, ERRNO_VALUE - 289, ERRNO_VALUE)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, IgnoringErrnosByMultipleIgnoreErrnosCallsWorksWhenErrnoIsFirst)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 117;
    constexpr int ERRNO_VALUE = 118;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .ignoreErrnos(ERRNO_VALUE)
        .ignoreErrnos(ERRNO_VALUE - 10)
        .ignoreErrnos(ERRNO_VALUE + 17)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, IgnoringErrnosByMultipleIgnoreErrnosCallsWorksWhenErrnoIsMiddle)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 217;
    constexpr int ERRNO_VALUE = 218;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .ignoreErrnos(ERRNO_VALUE - 10)
        .ignoreErrnos(ERRNO_VALUE)
        .ignoreErrnos(ERRNO_VALUE + 17)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, IgnoringErrnosByMultipleIgnoreErrnosCallsWorksWhenErrnoIsLast)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 317;
    constexpr int ERRNO_VALUE = 318;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .ignoreErrnos(ERRNO_VALUE - 10)
        .ignoreErrnos(ERRNO_VALUE + 17)
        .ignoreErrnos(ERRNO_VALUE)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, IgnoringErrnosByMultipleIgnoreErrnosCallsFails)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 417;
    constexpr int ERRNO_VALUE = 418;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .ignoreErrnos(ERRNO_VALUE - 10)
        .ignoreErrnos(ERRNO_VALUE + 13)
        .ignoreErrnos(ERRNO_VALUE + 17)
        .evaluate()
        .and_then([&](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, SuppressErrnoLoggingWithNonPresentErrnoPrintsErrorMessage)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 111;
    constexpr int ERRNO_VALUE = 112;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .suppressErrorMessagesForErrnos(ERRNO_VALUE - 10)
        .evaluate()
        .and_then([&](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, SuppressErrnoLoggingWithPresentErrnoDoesNotPrintErrorMessage)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 113;
    constexpr int ERRNO_VALUE = 114;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .suppressErrorMessagesForErrnos(ERRNO_VALUE)
        .evaluate()
        .and_then([&](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, SuppressMultipleErrnoLoggingWithNoPresentErrnoPrintsErrorMessage)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 115;
    constexpr int ERRNO_VALUE = 116;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .suppressErrorMessagesForErrnos(ERRNO_VALUE - 10, ERRNO_VALUE + 16, ERRNO_VALUE + 17)
        .evaluate()
        .and_then([&](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, SuppressMultipleErrnoLoggingWithPresentErrnoDoesNotPrintErrorMessage)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 117;
    constexpr int ERRNO_VALUE = 118;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .suppressErrorMessagesForErrnos(ERRNO_VALUE - 10, ERRNO_VALUE, ERRNO_VALUE + 17)
        .evaluate()
        .and_then([&](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, SuppressErrnoLoggingByMultipleCallsWithNonPresentErrnoPrintsErrorMessage)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 119;
    constexpr int ERRNO_VALUE = 120;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .suppressErrorMessagesForErrnos(ERRNO_VALUE - 10)
        .suppressErrorMessagesForErrnos(ERRNO_VALUE + 13)
        .suppressErrorMessagesForErrnos(ERRNO_VALUE + 17)
        .evaluate()
        .and_then([&](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, SuppressErrnoLoggingByMultipleCallsWithPresentErrnoDoesNotPrintErrorMessage)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 121;
    constexpr int ERRNO_VALUE = 122;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .suppressErrorMessagesForErrnos(ERRNO_VALUE - 10)
        .suppressErrorMessagesForErrnos(ERRNO_VALUE)
        .suppressErrorMessagesForErrnos(ERRNO_VALUE + 17)
        .evaluate()
        .and_then([&](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, SuppressErrnoLoggingOfIgnoredErrnoDoesNotPrintErrorMessage)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 123;
    constexpr int ERRNO_VALUE = 124;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .ignoreErrnos(ERRNO_VALUE)
        .suppressErrorMessagesForErrnos(ERRNO_VALUE)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([&](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, SuppressErrnoLoggingOfNotIgnoredErrnoDoesNotPrintErrorMessage)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 123;
    constexpr int ERRNO_VALUE = 124;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(1)
        .ignoreErrnos(ERRNO_VALUE + 10)
        .suppressErrorMessagesForErrnos(ERRNO_VALUE)
        .evaluate()
        .and_then([&](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, RecallingFunctionWithEintrWorks)
{
    internal::CaptureStderr();

    eintrRepetition = iox::posix::POSIX_CALL_EINTR_REPETITIONS;
    iox::posix::posixCall(testEintr)()
        .successReturnValue(0)
        .evaluate()
        .and_then([](auto& r) {
            EXPECT_THAT(r.value, Eq(0));
            EXPECT_THAT(r.errnum, Eq(0));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_THAT(eintrRepetition, Eq(0));
    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}


TEST_F(PosixCall_test, FunctionReturnsEINTRTooOftenResultsInFailure)
{
    internal::CaptureStderr();

    eintrRepetition = iox::posix::POSIX_CALL_EINTR_REPETITIONS + 1;
    iox::posix::posixCall(testEintr)()
        .successReturnValue(0)
        .evaluate()
        .and_then([](auto&) { EXPECT_TRUE(false); })
        .or_else([](auto& r) {
            EXPECT_THAT(r.value, Eq(1));
            EXPECT_THAT(r.errnum, Eq(EINTR));
        });

    EXPECT_THAT(eintrRepetition, Eq(1));
    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithMultipleSuccessReturnValuesWhereGoodValueIsFirst)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 25;
    constexpr int ERRNO_VALUE = 26;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(RETURN_VALUE, RETURN_VALUE - 1, RETURN_VALUE + 1, RETURN_VALUE + 2)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithMultipleSuccessReturnValuesWhereGoodValueIsCenter)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 27;
    constexpr int ERRNO_VALUE = 28;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(RETURN_VALUE - 1, RETURN_VALUE + 1, RETURN_VALUE, RETURN_VALUE + 2)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithMultipleSuccessReturnValuesWhereGoodValueIsLast)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 29;
    constexpr int ERRNO_VALUE = 30;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(RETURN_VALUE - 1, RETURN_VALUE + 1, RETURN_VALUE + 2, RETURN_VALUE)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithMultipleSuccessReturnValuesWhereGoodValueIsNotPresent)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 31;
    constexpr int ERRNO_VALUE = 32;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .successReturnValue(RETURN_VALUE - 1, RETURN_VALUE + 1, RETURN_VALUE + 2)
        .evaluate()
        .and_then([&](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithMultipleFailureReturnValuesWhereFailureValueIsFirst)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 33;
    constexpr int ERRNO_VALUE = 34;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .failureReturnValue(RETURN_VALUE, RETURN_VALUE - 1, RETURN_VALUE + 1, RETURN_VALUE + 2)
        .evaluate()
        .and_then([&](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithMultipleFailureReturnValuesWhereFailureValueIsCenter)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 35;
    constexpr int ERRNO_VALUE = 36;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .failureReturnValue(RETURN_VALUE - 1, RETURN_VALUE, RETURN_VALUE + 1, RETURN_VALUE + 2)
        .evaluate()
        .and_then([&](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithMultipleFailureReturnValuesWhereFailureValueIsLast)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 37;
    constexpr int ERRNO_VALUE = 38;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .failureReturnValue(RETURN_VALUE - 1, RETURN_VALUE + 1, RETURN_VALUE + 2, RETURN_VALUE)
        .evaluate()
        .and_then([&](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        });

    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithMultipleFailureReturnValuesWhereFailureValueIsNotPresent)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 39;
    constexpr int ERRNO_VALUE = 40;

    iox::posix::posixCall(testFunction)(RETURN_VALUE, ERRNO_VALUE)
        .failureReturnValue(RETURN_VALUE - 1, RETURN_VALUE + 1, RETURN_VALUE + 2)
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(ERRNO_VALUE));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, ErrnoIsSetFromReturnValueWhenFunctionHandlesErrnosInReturnValue_GoodCase)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 0;

    iox::posix::posixCall(returnValueIsErrno)(RETURN_VALUE)
        .returnValueMatchesErrno()
        .evaluate()
        .and_then([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(0));
        })
        .or_else([&](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, ErrnoIsSetFromReturnValueWhenFunctionHandlesErrnosInReturnValue_BadCase)
{
    internal::CaptureStderr();

    constexpr int RETURN_VALUE = 42;

    iox::posix::posixCall(returnValueIsErrno)(RETURN_VALUE)
        .returnValueMatchesErrno()
        .evaluate()
        .and_then([&](auto&) { EXPECT_TRUE(false); })
        .or_else([&](auto& r) {
            EXPECT_THAT(r.value, Eq(RETURN_VALUE));
            EXPECT_THAT(r.errnum, Eq(RETURN_VALUE));
        });

    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}
