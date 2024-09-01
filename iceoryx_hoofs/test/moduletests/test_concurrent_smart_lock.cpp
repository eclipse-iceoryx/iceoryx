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

#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iox/atomic.hpp"
#include "iox/optional.hpp"
#include "iox/smart_lock.hpp"
#include "iox/vector.hpp"
#include "test.hpp"

#include <thread>

using namespace ::testing;
using namespace iox::concurrent;

namespace
{
class SmartLockTester
{
  public:
    // just for clarity, we could also write ++m_a but we want
    // to highlight the possible race condition here when we call this from
    // multiple threads, therefore m_a = m_a + 1
    SmartLockTester() noexcept = default;

    explicit SmartLockTester(const int32_t a) noexcept
        : m_a(a)
    {
    }

    SmartLockTester(const SmartLockTester& rhs) noexcept
        : m_a(rhs.m_a)
    {
        // NOLINTNEXTLINE(cert-oop58-cpp) used to test thread safety of smart_lock
        rhs.m_b = rhs.m_b + 1;
    }

    SmartLockTester(SmartLockTester&& rhs) noexcept
        : m_a(rhs.m_a)
    {
        rhs.m_isMoved = true;
        rhs.m_a = 0;
        rhs.m_b = rhs.m_b + 1;
    }

    SmartLockTester& operator=(const SmartLockTester& rhs) noexcept
    {
        if (this != &rhs)
        {
            // NOLINTNEXTLINE(cert-oop58-cpp) used to test thread safety of smart_lock
            rhs.m_b = rhs.m_b + 1;
            m_a = rhs.m_a;
        }
        return *this;
    }

    SmartLockTester& operator=(SmartLockTester&& rhs) noexcept
    {
        if (this != &rhs)
        {
            rhs.m_b = rhs.m_b + 1;
            m_a = rhs.m_a;
            rhs.m_a = 0;
            rhs.m_isMoved = true;
        }
        return *this;
    }

    ~SmartLockTester() noexcept = default;

    int32_t getA() const
    {
        return m_a;
    }

    int32_t getB() const
    {
        return m_b;
    }

    void incrementA()
    {
        m_a = m_a + 1;
    }

    void constIncrementA() const
    {
        m_a = m_a + 1;
    }

    bool isMoved() const
    {
        return m_isMoved;
    }

  private:
    mutable int32_t m_a = 0;
    mutable int32_t m_b = 0;
    bool m_isMoved = false;
};

class smart_lock_test : public Test
{
  public:
    void SetUp() override
    {
        m_watchdog.watchAndActOnFailure([] { std::terminate(); });
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

    Watchdog m_watchdog{iox::units::Duration::fromSeconds(60U)};
    using SutType_t = smart_lock<SmartLockTester>;
    iox::optional<SutType_t> m_sut;
    iox::concurrent::Atomic<uint64_t> m_numberOfThreadWaiter{0U};
};
constexpr uint64_t NUMBER_OF_RUNS_PER_THREAD = 100000U;
constexpr uint64_t NUMBER_OF_THREADS = 4;

} // namespace

//////////////////////////////////////
// BEGIN single threaded api test
//////////////////////////////////////

TEST_F(smart_lock_test, DefaultConstructionOfUnderlyingObjectWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "7b70e351-0e34-4eed-ba02-fcea2b95aaa1");
    m_sut.emplace();
    EXPECT_THAT((*m_sut)->getA(), Eq(0));
}

TEST_F(smart_lock_test, ConstructionWithOneValueCTorOfUnderlyingObjectWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1c464484-7e64-421a-84f0-1b399586deea");
    constexpr int32_t CTOR_VALUE = 25;
    m_sut.emplace(ForwardArgsToCTor, CTOR_VALUE);
    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, CopyConstructionOfUnderlyinObjectWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3c03837e-c5f9-4937-a5d3-77a507fe91a5");
    constexpr int32_t CTOR_VALUE = 121;
    SmartLockTester tester(CTOR_VALUE);
    m_sut.emplace(ForwardArgsToCTor, tester);
    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
    EXPECT_THAT(tester.getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, MoveConstructionOfUnderlyinObjectWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "8ded7285-3608-4e55-b9e9-a97fb8b1345e");
    constexpr int32_t CTOR_VALUE = 1211;
    SmartLockTester tester(CTOR_VALUE);
    m_sut.emplace(ForwardArgsToCTor, std::move(tester));
    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
    // NOLINTJUSTIFICATION we want to test defined behavior of a moved smart_lock
    // NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    EXPECT_TRUE(tester.isMoved());
}

TEST_F(smart_lock_test, CopyConstructorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "04692305-6a72-45c4-bc4e-fc0cd8ed8dd7");
    constexpr int32_t CTOR_VALUE = 1221;
    m_sut.emplace(ForwardArgsToCTor, CTOR_VALUE);

    SutType_t sut2(*m_sut);

    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
    EXPECT_THAT(sut2->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, CopyAssignmentWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f8d68a06-00c5-4b68-a6b1-109b9d011ae8");
    constexpr int32_t CTOR_VALUE = 2121;
    m_sut.emplace(ForwardArgsToCTor, CTOR_VALUE);

    SutType_t sut2;
    sut2 = m_sut.value();

    EXPECT_THAT((*m_sut)->getA(), Eq(CTOR_VALUE));
    EXPECT_THAT(sut2->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, MoveConstructorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1a22291e-3e78-4335-9a9f-e704b767085f");
    constexpr int32_t CTOR_VALUE = 41221;
    m_sut.emplace(ForwardArgsToCTor, CTOR_VALUE);

    SutType_t sut2(std::move(*m_sut));

    EXPECT_TRUE((*m_sut)->isMoved());
    EXPECT_THAT(sut2->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, MoveAssignmentWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "82342f30-d261-4611-a972-4acaeae6379f");
    constexpr int32_t CTOR_VALUE = 21281;
    m_sut.emplace(ForwardArgsToCTor, CTOR_VALUE);

    SutType_t sut2;
    sut2 = std::move(m_sut.value());

    EXPECT_TRUE((*m_sut)->isMoved());
    EXPECT_THAT(sut2->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, ConstArrowOperatorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "9d46c335-48e6-424a-aedb-128b124f7cc3");
    constexpr int32_t CTOR_VALUE = 212818;
    const SutType_t constSut(ForwardArgsToCTor, CTOR_VALUE);

    EXPECT_THAT(constSut->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, AccessThroughConstScopeGuardWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f52e9b24-0819-487b-947a-5f18d24b55e6");
    constexpr int32_t CTOR_VALUE = 6212818;
    const SutType_t constSut(ForwardArgsToCTor, CTOR_VALUE);
    const auto guard = constSut.get_scope_guard();

    EXPECT_THAT(guard->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, AccessThroughScopeGuardWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "9d91a04f-d84e-4fa3-9ee0-95ebc4bfbed1");
    constexpr int32_t CTOR_VALUE = 62818;
    SutType_t Sut(ForwardArgsToCTor, CTOR_VALUE);
    auto guard = Sut.get_scope_guard();

    EXPECT_THAT(guard->getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, AccessViaConstDereferenceOperatorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "43db4ae1-86ad-4d36-93c4-c38f1faf68b5");
    constexpr int32_t CTOR_VALUE = 8182126;
    const SutType_t constSut(ForwardArgsToCTor, CTOR_VALUE);
    const auto guard = constSut.get_scope_guard();

    EXPECT_THAT((*guard).getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, AccessViaDereferenceOperatorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "45cccede-a609-493d-a285-20c1dfb77714");
    constexpr int32_t CTOR_VALUE = 81826;
    SutType_t Sut(ForwardArgsToCTor, CTOR_VALUE);
    auto guard = Sut.get_scope_guard();

    EXPECT_THAT((*guard).getA(), Eq(CTOR_VALUE));
}

TEST_F(smart_lock_test, AcquiringCopyWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "47863174-e936-4b18-9617-c58bb956ccac");
    constexpr int32_t CTOR_VALUE = 628189;
    m_sut.emplace(ForwardArgsToCTor, CTOR_VALUE);

    EXPECT_THAT(m_sut->get_copy().getA(), Eq(CTOR_VALUE));
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

    iox::vector<std::thread, NUMBER_OF_THREADS> threads;
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

TEST_F(smart_lock_test, ThreadSafeAccessThroughArrowOperator)
{
    ::testing::Test::RecordProperty("TEST_ID", "e6cfd20a-4450-46f7-80ca-b87f75a0462e");
    threadSafeOperationTest(this, [this] { (*m_sut)->incrementA(); });

    EXPECT_THAT((*m_sut)->getA(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, ThreadSafeAccessThroughConstArrowOperator)
{
    ::testing::Test::RecordProperty("TEST_ID", "c0dedded-cf07-47dc-9032-e5de3db859d2");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) used to explitly test const arrow operator
    threadSafeOperationTest(this, [this] { (const_cast<const SutType_t&>(*m_sut))->constIncrementA(); });
    EXPECT_THAT((*m_sut)->getA(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, ThreadSafeAccessThroughScopedGuard)
{
    ::testing::Test::RecordProperty("TEST_ID", "2ffb9ba4-4df3-4851-8976-20f15a3bfa4b");
    threadSafeOperationTest(this, [this] {
        auto guard = (*m_sut).get_scope_guard();
        guard->incrementA();
    });
    EXPECT_THAT((*m_sut)->getA(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, ThreadSafeAccessThroughConstScopedGuard)
{
    ::testing::Test::RecordProperty("TEST_ID", "dc2efd41-4c0a-4ac0-93ed-f8e9195e0dfd");
    threadSafeOperationTest(this, [this] {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) used to explitly test const get_scope_guard()
        auto guard = const_cast<const SutType_t&>(*m_sut).get_scope_guard();
        guard->incrementA();
    });
    EXPECT_THAT((*m_sut)->getA(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, ThreadSafeCopyCTor)
{
    ::testing::Test::RecordProperty("TEST_ID", "23b27eda-17de-42b9-bdbc-81e7bae15fd6");
    threadSafeOperationTest(this, [this] { SutType_t someCopy(*m_sut); });
    EXPECT_THAT((*m_sut)->getB(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, ThreadSafeMoveCTor)
{
    ::testing::Test::RecordProperty("TEST_ID", "e7368392-ff41-44a7-ad58-9d3b17637dd6");
    threadSafeOperationTest(this, [this] { SutType_t movedSut(std::move(*m_sut)); });
    EXPECT_THAT((*m_sut)->getB(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, ThreadSafeCopyAssignment)
{
    ::testing::Test::RecordProperty("TEST_ID", "a9964501-9c88-4250-a1d5-e266171a670c");
    threadSafeOperationTest(this, [this] {
        SutType_t someCopy;
        someCopy = *m_sut;
    });
    EXPECT_THAT((*m_sut)->getB(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

TEST_F(smart_lock_test, ThreadSafeMoveAssignment)
{
    ::testing::Test::RecordProperty("TEST_ID", "3f89e951-bf93-4230-b749-ec91416785ae");
    threadSafeOperationTest(this, [this] {
        SutType_t someMovedSut;
        someMovedSut = std::move(*m_sut);
    });
    EXPECT_THAT((*m_sut)->getB(), Eq(NUMBER_OF_RUNS_PER_THREAD * NUMBER_OF_THREADS));
}

//////////////////////////////////////
// END thread safety tests
//////////////////////////////////////
