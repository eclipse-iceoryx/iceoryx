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
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/internal/concurrent/smart_lock.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"
#include "test.hpp"

#include <atomic>
#include <thread>

using namespace ::testing;
using namespace iox::concurrent;
using namespace iox::cxx;

namespace
{
class SmartLockTester
{
  public:
    // just for clarity, we could also write ++m_a but we want
    // to highlight the possible race condition here when we call this from
    // multiple threads, therefore m_a = m_a + 1
    SmartLockTester()
    {
    }

    SmartLockTester(const int32_t a)
        : m_a(a)
    {
    }

    SmartLockTester(const SmartLockTester& rhs)
        : m_a(rhs.m_a)
    {
        rhs.m_b = rhs.m_b + 1;
    }

    SmartLockTester(SmartLockTester&& rhs)
        : m_a(rhs.m_a)
    {
        rhs.m_a = 0;
        rhs.m_b = rhs.m_b + 1;
    }

    SmartLockTester& operator=(const SmartLockTester& rhs)
    {
        if (this != &rhs)
        {
            rhs.m_b = rhs.m_b + 1;
            m_a = rhs.m_a;
        }
        return *this;
    }

    SmartLockTester& operator=(SmartLockTester&& rhs)
    {
        if (this != &rhs)
        {
            rhs.m_b = rhs.m_b + 1;
            m_a = rhs.m_a;
            rhs.m_a = 0;
        }
        return *this;
    }

    ~SmartLockTester()
    {
    }

    int32_t getA() const
    {
        return m_a;
    }

    int32_t getB() const
    {
        return m_b;
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

  private:
    mutable int32_t m_a = 0;
    mutable int32_t m_b = 0;
};

class smart_lock_test : public Test
{
  public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }

    void waitUntilThreadsHaveStarted(const uint64_t numberOfThreads)
    {
        ++m_numberOfThreadWaiter;
        while (m_numberOfThreadWaiter.load() != numberOfThreads)
        {
        }
    }

    using SutType_t = smart_lock<SmartLockTester>;
    optional<SutType_t> m_sut;
    std::atomic<uint64_t> m_numberOfThreadWaiter{0U};
};
constexpr uint64_t NUMBER_OF_RUNS_PER_THREAD = 100000U;
constexpr uint64_t NUMBER_OF_THREADS = 4;

} // namespace

//////////////////////////////////////
// BEGIN single threaded api test
//////////////////////////////////////

TEST_F(smart_lock_test, defaultConstructionOfUnderlyingObjectWorks)
{
    m_sut.emplace();
    EXPECT_THAT((*m_sut)->getA(), Eq(0));
}

TEST_F(smart_lock_test, constructionWithOneValueCTorOfUnderlyingObjectWorks)
{
    constexpr int32_t CTOR_VALUE = 25;
    m_sut.emplace(ForwardArgsToCTor, CTOR_VALUE);
    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, copyConstructionOfUnderlyinObjectWorks)
{
    constexpr int32_t CTOR_VALUE = 121;
    SmartLockTester tester(CTOR_VALUE);
    m_sut.emplace(ForwardArgsToCTor, tester);
    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
    EXPECT_THAT(tester.getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, moveConstructionOfUnderlyinObjectWorks)
{
    constexpr int32_t CTOR_VALUE = 1211;
    SmartLockTester tester(CTOR_VALUE);
    m_sut.emplace(ForwardArgsToCTor, std::move(tester));
    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
    EXPECT_THAT(tester.getA(), Eq(0));
}

TEST_F(smart_lock_test, copyConstructorWorks)
{
    constexpr int32_t CTOR_VALUE = 1221;
    m_sut.emplace(ForwardArgsToCTor, CTOR_VALUE);

    SutType_t sut2(*m_sut);

    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
    EXPECT_THAT(sut2->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, copyAssignmentWorks)
{
    constexpr int32_t CTOR_VALUE = 2121;
    m_sut.emplace(ForwardArgsToCTor, CTOR_VALUE);

    SutType_t sut2;
    sut2 = m_sut.value();

    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
    EXPECT_THAT(sut2->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, moveConstructorWorks)
{
    constexpr int32_t CTOR_VALUE = 41221;
    m_sut.emplace(ForwardArgsToCTor, CTOR_VALUE);

    SutType_t sut2(std::move(*m_sut));

    EXPECT_THAT((*m_sut)->getA(), Eq(0));
    EXPECT_THAT(sut2->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, moveAssignmentWorks)
{
    constexpr int32_t CTOR_VALUE = 21281;
    m_sut.emplace(ForwardArgsToCTor, CTOR_VALUE);

    SutType_t sut2;
    sut2 = std::move(m_sut.value());

    EXPECT_THAT((*m_sut)->getA(), Eq(0));
    EXPECT_THAT(sut2->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, constArrowOperatorWorks)
{
    constexpr int32_t CTOR_VALUE = 212818;
    const SutType_t constSut(ForwardArgsToCTor, CTOR_VALUE);

    EXPECT_THAT(constSut->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, accessThroughConstScopeGuardWorks)
{
    constexpr int32_t CTOR_VALUE = 6212818;
    const SutType_t constSut(ForwardArgsToCTor, CTOR_VALUE);
    auto guard = constSut.getScopeGuard();

    EXPECT_THAT(guard->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, accessThroughScopeGuardWorks)
{
    constexpr int32_t CTOR_VALUE = 62818;
    SutType_t constSut(ForwardArgsToCTor, CTOR_VALUE);
    auto guard = constSut.getScopeGuard();

    EXPECT_THAT(guard->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, acquiringCopyWorks)
{
    constexpr int32_t CTOR_VALUE = 628189;
    m_sut.emplace(ForwardArgsToCTor, CTOR_VALUE);

    EXPECT_THAT(m_sut->getCopy().getA(), Eq(CTOR_VALUE));
}

//////////////////////////////////////
// END single threaded api test
//////////////////////////////////////

//////////////////////////////////////
// BEGIN thread safety tests
//////////////////////////////////////

// the idea of all tests when the testAction incrementA or constIncrementA
// would not be thread safe we would encounter either undefined behavior since
// the variable is sometimes written while it is read or we face a race condition
// since the increment is always number = number + 1. If this is not thread safe
// we encounter a jumping in the number and not a steady increment.
//
// If no such thing occurs, since the operation is performed in a threadsafe
// manner, (*m_sut)->getA() == totalSumOfIncrements
void threadSafeOperationTest(smart_lock_test* test, const std::function<void()> testAction)
{
    test->m_sut.emplace(ForwardArgsToCTor, 0);

    vector<std::thread, NUMBER_OF_THREADS> threads;
    for (uint64_t i = 0U; i < NUMBER_OF_THREADS; ++i)
    {
        threads.emplace_back([&] {
            test->waitUntilThreadsHaveStarted(NUMBER_OF_THREADS);
            for (uint64_t k = 0U; k < NUMBER_OF_RUNS_PER_THREAD; ++k)
            {
                testAction();
            }
        });
    }

    for (uint64_t i = 0U; i < NUMBER_OF_THREADS; ++i)
    {
        threads[i].join();
    }
}

TEST_F(smart_lock_test, threadSafeAccessThroughArrowOperator)
{
    threadSafeOperationTest(this, [=] { (*m_sut)->incrementA(); });

    EXPECT_THAT((*m_sut)->getA(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, threadSafeAccessThroughConstArrowOperator)
{
    threadSafeOperationTest(this, [=] { (const_cast<const SutType_t&>(*m_sut))->constIncrementA(); });
    EXPECT_THAT((*m_sut)->getA(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, threadSafeAccessThroughScopedGuard)
{
    threadSafeOperationTest(this, [=] {
        auto guard = (*m_sut).getScopeGuard();
        guard->incrementA();
    });
    EXPECT_THAT((*m_sut)->getA(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, threadSafeAccessThroughConstScopedGuard)
{
    threadSafeOperationTest(this, [=] {
        auto guard = const_cast<const SutType_t&>(*m_sut).getScopeGuard();
        guard->incrementA();
    });
    EXPECT_THAT((*m_sut)->getA(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, threadSafeCopyCTor)
{
    threadSafeOperationTest(this, [=] { SutType_t someCopy(*m_sut); });
    EXPECT_THAT((*m_sut)->getB(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, threadSafeMoveCTor)
{
    threadSafeOperationTest(this, [=] { SutType_t movedSut(std::move(*m_sut)); });
    EXPECT_THAT((*m_sut)->getB(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, threadSafeCopyAssignment)
{
    threadSafeOperationTest(this, [=] {
        SutType_t someCopy;
        someCopy = *m_sut;
    });
    EXPECT_THAT((*m_sut)->getB(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, threadSafeMoveAssignment)
{
    threadSafeOperationTest(this, [=] {
        SutType_t someCopy;
        someCopy = std::move(*m_sut);
    });
    EXPECT_THAT((*m_sut)->getB(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

//////////////////////////////////////
// END thread safety tests
//////////////////////////////////////
