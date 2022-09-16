// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_dust/singleton.hpp"

#include "test.hpp"
#include <iostream>

namespace
{
using namespace ::testing;

// intentionally not defined in Foo (cannot be used in tests there)
constexpr uint32_t DEFAULT_VALUE{66};

struct Foo
{
    Foo()
    {
        ++numDefaultCtorCalls;
    }

    explicit Foo(uint32_t v)
        : value(v)
    {
        ++numCtorCalls;
    }

    Foo(const Foo&) = delete;
    Foo(Foo&&) = delete;
    Foo& operator=(const Foo&) = delete;
    Foo& operator=(Foo&&) = delete;

    ~Foo()
    {
        ++numDtorCalls;
    }

    uint32_t value{DEFAULT_VALUE};

    static uint32_t numDefaultCtorCalls;
    static uint32_t numCtorCalls;
    static uint32_t numDtorCalls;

    static void reset()
    {
        numDefaultCtorCalls = 0;
        numCtorCalls = 0;
        numDtorCalls = 0;
    }
};

uint32_t Foo::numDefaultCtorCalls;
uint32_t Foo::numCtorCalls;
uint32_t Foo::numDtorCalls;


using TestSingleton = iox::Singleton<Foo>;

class Singleton_test : public Test
{
  public:
    void SetUp() override
    {
        internal::CaptureStderr();
        Foo::reset();
    }

    void TearDown() override
    {
        // - the tests cannot be fully independent due to the static construct
        // - we reset the singleton before each test with destroy, i.e.
        // rely on destroy to work (which is tested first on its own)
        TestSingleton::destroy();

        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
};

TEST_F(Singleton_test, destroy)
{
    TestSingleton::init();
    EXPECT_TRUE(TestSingleton::isInitialized());
    TestSingleton::destroy();

    EXPECT_FALSE(TestSingleton::isInitialized());
    EXPECT_EQ(Foo::numDtorCalls, 1);
}

TEST_F(Singleton_test, defaultInit)
{
    EXPECT_FALSE(TestSingleton::isInitialized());
    auto& foo = TestSingleton::init();

    EXPECT_EQ(foo.value, DEFAULT_VALUE);
    EXPECT_TRUE(TestSingleton::isInitialized());
    EXPECT_EQ(Foo::numDefaultCtorCalls, 1);
}

TEST_F(Singleton_test, initWithArguments)
{
    constexpr uint32_t VAL{73};
    EXPECT_FALSE(TestSingleton::isInitialized());
    auto& foo = TestSingleton::init(VAL);

    EXPECT_EQ(foo.value, VAL);
    EXPECT_TRUE(TestSingleton::isInitialized());
    EXPECT_EQ(TestSingleton::instance().value, VAL);
    EXPECT_EQ(Foo::numCtorCalls, 1);
}

TEST_F(Singleton_test, multiDestroyDoesCallDtorOnce)
{
    TestSingleton::init();
    TestSingleton::destroy();
    EXPECT_FALSE(TestSingleton::isInitialized());
    TestSingleton::destroy();

    EXPECT_FALSE(TestSingleton::isInitialized());
    EXPECT_EQ(Foo::numDtorCalls, 1);
}

TEST_F(Singleton_test, reinitAfterDestroy)
{
    constexpr uint32_t VAL{73};
    TestSingleton::init();
    EXPECT_EQ(Foo::numDefaultCtorCalls, 1);
    TestSingleton::destroy();
    TestSingleton::init(VAL);

    EXPECT_EQ(TestSingleton::instance().value, VAL);
    EXPECT_EQ(Foo::numDefaultCtorCalls, 1);
    EXPECT_EQ(Foo::numCtorCalls, 1);
    EXPECT_EQ(Foo::numDtorCalls, 1);
}

TEST_F(Singleton_test, nonInitDestroyDoesNotCallDtor)
{
    TestSingleton::destroy();

    EXPECT_FALSE(TestSingleton::isInitialized());
    EXPECT_EQ(Foo::numDtorCalls, 0);
}

TEST_F(Singleton_test, nonInitInstanceCallsDefaultCtor)
{
    auto& foo = TestSingleton::instance();

    EXPECT_TRUE(TestSingleton::isInitialized());
    EXPECT_EQ(Foo::numDefaultCtorCalls, 1);
    EXPECT_EQ(foo.value, DEFAULT_VALUE);
}

TEST_F(Singleton_test, initInstanceCallsNoCtor)
{
    constexpr uint32_t VAL{73};
    TestSingleton::init(VAL);
    EXPECT_EQ(Foo::numCtorCalls, 1);
    auto& foo = TestSingleton::instance();

    EXPECT_TRUE(TestSingleton::isInitialized());
    EXPECT_EQ(Foo::numCtorCalls, 1);
    EXPECT_EQ(Foo::numDefaultCtorCalls, 0);
    EXPECT_EQ(foo.value, VAL);
}

TEST_F(Singleton_test, initAfterInstanceCallsNoCtor)
{
    constexpr uint32_t VAL{73};
    auto& foo = TestSingleton::instance();
    EXPECT_TRUE(TestSingleton::isInitialized());
    EXPECT_EQ(Foo::numDefaultCtorCalls, 1);
    EXPECT_EQ(Foo::numCtorCalls, 0);

    TestSingleton::init(VAL);
    EXPECT_EQ(Foo::numDefaultCtorCalls, 1);
    EXPECT_EQ(Foo::numCtorCalls, 0);
    EXPECT_EQ(foo.value, DEFAULT_VALUE);
}

TEST_F(Singleton_test, multiInstanceCallsDefaultCtorOnce)
{
    TestSingleton::instance();
    auto& foo = TestSingleton::instance();

    EXPECT_EQ(foo.value, DEFAULT_VALUE);
    EXPECT_EQ(Foo::numDefaultCtorCalls, 1);
    EXPECT_EQ(Foo::numCtorCalls, 0);
    EXPECT_EQ(Foo::numDtorCalls, 0);
}

// Note: automatic destruction of wrapped Foo in Singleton<Foo> after main cannot be checked
// (cannot run tests after main)
// verify by review that destroy is called in Singleton<Foo> dtor which calls the dtor unless not initialized

} // namespace