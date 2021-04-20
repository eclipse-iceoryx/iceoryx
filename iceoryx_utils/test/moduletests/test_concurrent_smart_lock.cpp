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

#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/internal/concurrent/smart_lock.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::concurrent;
using namespace iox::cxx;

namespace
{
class SmartLockTester
{
  public:
    SmartLockTester()
    {
        m_callCounter = m_callCounter + 1;
    }

    SmartLockTester(const int32_t a)
        : m_a(a)
    {
        m_callCounter = m_callCounter + 1;
    }

    SmartLockTester(const SmartLockTester& rhs)
        : m_a(rhs.m_a)
    {
        m_callCounter = m_callCounter + 1;
    }

    SmartLockTester(SmartLockTester&& rhs)
        : m_a(rhs.m_a)
    {
        m_callCounter = m_callCounter + 1;
        rhs.m_a = 0;
    }

    SmartLockTester& operator=(const SmartLockTester& rhs)
    {
        m_callCounter = m_callCounter + 1;
        if (this != &rhs)
        {
            m_a = rhs.m_a;
        }
        return *this;
    }

    SmartLockTester& operator=(SmartLockTester&& rhs)
    {
        m_callCounter = m_callCounter + 1;
        if (this != &rhs)
        {
            m_a = rhs.m_a;
            rhs.m_a = 0;
        }
        return *this;
    }

    ~SmartLockTester()
    {
        m_callCounter = m_callCounter + 1;
    }

    int32_t getA() const
    {
        return m_a;
    }

    void setA(const int32_t a)
    {
        m_a = a;
    }

    void incrementA()
    {
        m_a = m_a + 1;
    }

    void constIncrementA() const
    {
        m_a = m_a + 1;
    }

    static int32_t m_callCounter;

  private:
    mutable int32_t m_a = 0;
};
int32_t SmartLockTester::m_callCounter = 0;

class smart_lock_test : public Test
{
  public:
    void SetUp() override
    {
        SmartLockTester::m_callCounter = 0;
    }
    void TearDown() override
    {
    }

    using SutType_t = smart_lock<SmartLockTester>;
    optional<SutType_t> m_sut;
};
} // namespace

TEST_F(smart_lock_test, defaultConstructionOfUnderlyingObjectWorks)
{
    m_sut.emplace();
    EXPECT_THAT((*m_sut)->getA(), Eq(0));
}

TEST_F(smart_lock_test, constructionWithOneValueCTorOfUnderlyingObjectWorks)
{
    constexpr int32_t CTOR_VALUE = 25;
    m_sut.emplace(CTOR_VALUE);
    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, copyConstructionOfUnderlyinObjectWorks)
{
    constexpr int32_t CTOR_VALUE = 121;
    SmartLockTester tester(CTOR_VALUE);
    m_sut.emplace(tester);
    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
    EXPECT_THAT(tester.getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, moveConstructionOfUnderlyinObjectWorks)
{
    constexpr int32_t CTOR_VALUE = 121;
    SmartLockTester tester(CTOR_VALUE);
    m_sut.emplace(std::move(tester));
    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
    EXPECT_THAT(tester.getA(), Eq(0));
}

TEST_F(smart_lock_test, copyConstructorWorks)
{
#if 0
    constexpr int32_t CTOR_VALUE = 121;
    m_sut.emplace(CTOR_VALUE);

    SutType_t sut2(*m_sut);

    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
    EXPECT_THAT(sut2->getA(), Eq(CTOR_VALUE));
#endif
}
