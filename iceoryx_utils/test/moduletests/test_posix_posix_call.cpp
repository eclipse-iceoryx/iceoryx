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

#include "iceoryx_utils/posix_wrapper/posix_call.hpp"
#include "test.hpp"

using namespace ::testing;

namespace
{
int testFunction(int a, int b)
{
    errno = a * b;
    return a + b;
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

class PosixCall_test : public Test
{
  public:
};
} // namespace

TEST_F(PosixCall_test, CallingFunctionWithSuccessReturnValue_GoodCase)
{
    internal::CaptureStderr();

    iox::posix::posixCall(testFunction)
        .call(1, 2)
        .successReturnValue(3)
        .evaluate()
        .and_then([](auto& r) {
            EXPECT_THAT(r.value, Eq(3));
            EXPECT_THAT(r.errnum, Eq(2));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithSuccessReturnValue_BadCase)
{
    internal::CaptureStderr();

    iox::posix::posixCall(testFunction)
        .call(2, 3)
        .successReturnValue(4)
        .evaluate()
        .and_then([](auto&) { EXPECT_TRUE(false); })
        .or_else([](auto& r) {
            EXPECT_THAT(r.value, Eq(5));
            EXPECT_THAT(r.errnum, Eq(6));
        });

    // we expect an error message via stderr to the console, details are not
    // verified since it depends on the target and where the source code is
    // stored
    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithFailureReturnValue_GoodCase)
{
    internal::CaptureStderr();

    iox::posix::posixCall(testFunction)
        .call(3, 4)
        .failureReturnValue(1)
        .evaluate()
        .and_then([](auto& r) {
            EXPECT_THAT(r.value, Eq(7));
            EXPECT_THAT(r.errnum, Eq(12));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithFailureReturnValue_BadCase)
{
    internal::CaptureStderr();

    iox::posix::posixCall(testFunction)
        .call(5, 6)
        .failureReturnValue(11)
        .evaluate()
        .and_then([](auto&) { EXPECT_TRUE(false); })
        .or_else([](auto& r) {
            EXPECT_THAT(r.value, Eq(11));
            EXPECT_THAT(r.errnum, Eq(30));
        });

    // we expect an error message via stderr to the console, details are not
    // verified since it depends on the target and where the source code is
    // stored
    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithSuccessReturnValueAndIgnoredErrno_GoodCase)
{
    internal::CaptureStderr();

    iox::posix::posixCall(testFunction)
        .call(7, 8)
        .successReturnValue(1)
        .evaluateWithIgnoredErrnos(56)
        .and_then([](auto& r) {
            EXPECT_THAT(r.value, Eq(15));
            EXPECT_THAT(r.errnum, Eq(56));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithSuccessReturnValueAndIgnoredErrno_BadCase)
{
    internal::CaptureStderr();

    iox::posix::posixCall(testFunction)
        .call(9, 10)
        .successReturnValue(1)
        .evaluateWithIgnoredErrnos(99)
        .and_then([](auto&) { EXPECT_TRUE(false); })
        .or_else([](auto& r) {
            EXPECT_THAT(r.value, Eq(19));
            EXPECT_THAT(r.errnum, Eq(90));
        });

    // we expect an error message via stderr to the console, details are not
    // verified since it depends on the target and where the source code is
    // stored
    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithFailureReturnValueAndIgnoredErrno_GoodCase)
{
    internal::CaptureStderr();

    iox::posix::posixCall(testFunction)
        .call(11, 12)
        .failureReturnValue(23)
        .evaluateWithIgnoredErrnos(132)
        .and_then([](auto& r) {
            EXPECT_THAT(r.value, Eq(23));
            EXPECT_THAT(r.errnum, Eq(132));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, CallingFunctionWithFailureReturnValueAndIgnoredErrno_BadCase)
{
    internal::CaptureStderr();

    iox::posix::posixCall(testFunction)
        .call(13, 14)
        .failureReturnValue(27)
        .evaluateWithIgnoredErrnos(1337)
        .and_then([](auto&) { EXPECT_TRUE(false); })
        .or_else([](auto& r) {
            EXPECT_THAT(r.value, Eq(27));
            EXPECT_THAT(r.errnum, Eq(182));
        });

    // we expect an error message via stderr to the console, details are not
    // verified since it depends on the target and where the source code is
    // stored
    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, IgnoringMultipleErrnosWorks)
{
    internal::CaptureStderr();

    iox::posix::posixCall(testFunction)
        .call(15, 16)
        .successReturnValue(1)
        .evaluateWithIgnoredErrnos(5, 240, 17)
        .and_then([](auto& r) {
            EXPECT_THAT(r.value, Eq(31));
            EXPECT_THAT(r.errnum, Eq(240));
        })
        .or_else([](auto&) { EXPECT_TRUE(false); });

    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(PosixCall_test, RecallingFunctionWithEintrWorks)
{
    internal::CaptureStderr();

    eintrRepetition = iox::posix::POSIX_CALL_EINTR_REPETITIONS;
    iox::posix::posixCall(testEintr)
        .call()
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
    iox::posix::posixCall(testEintr)
        .call()
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
