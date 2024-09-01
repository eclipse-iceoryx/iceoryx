// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iox/atomic.hpp"
#include "iox/signal_handler.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;

iox::concurrent::Atomic<int> signalOfCallback1{0};
iox::concurrent::Atomic<int> signalOfCallback2{0};

template <PosixSignal SignalValue>
struct SignalType
{
    static constexpr PosixSignal VALUE = SignalValue;
};

template <typename T>
class SignalHandler_test : public Test
{
  public:
    static constexpr PosixSignal SIGNAL_VALUE = T::VALUE;

    void SetUp() override
    {
        signalOfCallback1 = INVALID_SIGNAL;
        signalOfCallback2 = INVALID_SIGNAL;
    }

    void TearDown() override
    {
    }

    static void signalHandler1(int s)
    {
        signalOfCallback1 = s;
    }

    static void signalHandler2(int s)
    {
        signalOfCallback2 = s;
    }

    void registerSignal(const int signal, void (*const callback)(int))
    {
        struct sigaction action = {};
        sigemptyset(&action.sa_mask);
        // NOLINTJUSTIFICATION sigaction is a posix system construct, no other way to assign members
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        action.sa_handler = callback;
        action.sa_flags = 0;

        sigaction(signal, &action, nullptr);
    }

    static constexpr int INVALID_SIGNAL = std::numeric_limits<int>::max();
};

using Implementations = Types<SignalType<PosixSignal::INT>,
                              SignalType<PosixSignal::BUS>,
                              SignalType<PosixSignal::TERM>,
                              SignalType<PosixSignal::HUP>>;

TYPED_TEST_SUITE(SignalHandler_test, Implementations, );

TYPED_TEST(SignalHandler_test, RegisteringSignalGuardCallbackWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "7836be02-28ab-43b7-b7a7-7c43c4830eb4");
    PosixSignal signalValue = TestFixture::SIGNAL_VALUE;
    auto signalGuard [[maybe_unused]] = registerSignalHandler(signalValue, this->signalHandler1);

    ASSERT_EQ(raise(static_cast<int>(signalValue)), 0);

    EXPECT_THAT(signalOfCallback1.load(), Eq(static_cast<int>(signalValue)));
    EXPECT_THAT(signalOfCallback2.load(), Eq(this->INVALID_SIGNAL));
}

TYPED_TEST(SignalHandler_test, WhenSignalGuardGoesOutOfScopePreviousStateIsRestored)
{
    ::testing::Test::RecordProperty("TEST_ID", "8d2efc2b-837b-446d-ba16-fbca9a539b82");
    PosixSignal signalValue = TestFixture::SIGNAL_VALUE;
    this->registerSignal(static_cast<int>(signalValue), this->signalHandler2);
    {
        auto signalGuard [[maybe_unused]] = registerSignalHandler(signalValue, this->signalHandler1);
    }

    ASSERT_EQ(raise(static_cast<int>(signalValue)), 0);

    EXPECT_THAT(signalOfCallback1.load(), Eq(this->INVALID_SIGNAL));
    EXPECT_THAT(signalOfCallback2.load(), Eq(static_cast<int>(signalValue)));
}

TYPED_TEST(SignalHandler_test, MoveConstructedSignalGuardCallbackWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "8fcf886b-babb-41ab-a8ee-6ba123224aef");
    PosixSignal signalValue = TestFixture::SIGNAL_VALUE;
    auto signalGuard = registerSignalHandler(signalValue, this->signalHandler1);
    ASSERT_FALSE(signalGuard.has_error());

    SignalGuard signalGuard2(std::move(*signalGuard));

    ASSERT_EQ(raise(static_cast<int>(signalValue)), 0);

    EXPECT_THAT(signalOfCallback1.load(), Eq(static_cast<int>(signalValue)));
    EXPECT_THAT(signalOfCallback2.load(), Eq(this->INVALID_SIGNAL));
}

TYPED_TEST(SignalHandler_test, MoveConstructedSignalGuardRestoresPreviousState)
{
    ::testing::Test::RecordProperty("TEST_ID", "718c396f-ab2f-4ea3-bb90-f67f7ab131d8");
    PosixSignal signalValue = TestFixture::SIGNAL_VALUE;
    this->registerSignal(static_cast<int>(signalValue), this->signalHandler2);

    {
        auto signalGuard = registerSignalHandler(signalValue, this->signalHandler1);
        ASSERT_FALSE(signalGuard.has_error());
        SignalGuard signalGuard2(std::move(*signalGuard));
    }

    ASSERT_EQ(raise(static_cast<int>(signalValue)), 0);

    EXPECT_THAT(signalOfCallback1.load(), Eq(this->INVALID_SIGNAL));
    EXPECT_THAT(signalOfCallback2.load(), Eq(static_cast<int>(signalValue)));
}
} // namespace
