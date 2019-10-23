// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/posix_wrapper/semaphore.hpp"
#include "test.hpp"

#if not(defined(QNX) || defined(QNX__) || defined(__QNX__))

using namespace ::testing;

class Semaphore_test : public Test
{
  public:
    void SetUp()
    {
        internal::CaptureStderr();
    }
    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
};

TEST_F(Semaphore_test, CreateNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create("/fuuSem", S_IRUSR | S_IWUSR, 10);
    EXPECT_THAT(semaphore.has_error(), Eq(false));
}

TEST_F(Semaphore_test, CreateExistingNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create("/fuuSem1", S_IRUSR | S_IWUSR, 10);
    auto semaphore2 = iox::posix::Semaphore::create("/fuuSem1", S_IRUSR | S_IWUSR, 10);
    ASSERT_EQ(semaphore.has_error(), false);
    ASSERT_EQ(semaphore2.has_error(), true);
}

TEST_F(Semaphore_test, CreateLocalUnnamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create(10);
    EXPECT_THAT(semaphore.has_error(), Eq(false));
}

TEST_F(Semaphore_test, OpenNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create("/fuuSem", S_IRUSR | S_IWUSR, 10);
    auto semaphore2 = iox::posix::Semaphore::create("/fuuSem", S_IRUSR | S_IWUSR);
    EXPECT_THAT(semaphore.has_error(), Eq(false));
    EXPECT_THAT(semaphore2.has_error(), Eq(false));
}

TEST_F(Semaphore_test, OpenNonExistingNamedSemaphore)
{
    auto semaphore2 = iox::posix::Semaphore::create("/fuuSem", S_IRUSR | S_IWUSR);
    EXPECT_THAT(semaphore2.has_error(), Eq(true));
}

TEST_F(Semaphore_test, GetValueValidUnnamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create(1234);
    int i;
    EXPECT_THAT(semaphore->getValue(i), Eq(true));
    EXPECT_THAT(i, Eq(1234));
}

TEST_F(Semaphore_test, GetValueValidNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create("/fuuSem", S_IRUSR | S_IWUSR, 4337);
    int i;
    EXPECT_THAT(semaphore->getValue(i), Eq(true));
    EXPECT_THAT(i, Eq(4337));
}

TEST_F(Semaphore_test, PostValidUnnamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create(1234);
    EXPECT_THAT(semaphore->post(), Eq(true));
    EXPECT_THAT(semaphore->post(), Eq(true));
    EXPECT_THAT(semaphore->post(), Eq(true));

    int i;
    EXPECT_THAT(semaphore->getValue(i), Eq(true));
    EXPECT_THAT(i, Eq(1237));
}

TEST_F(Semaphore_test, PostValidNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create("/fuuSa", S_IRUSR | S_IWUSR, 42);
    EXPECT_THAT(semaphore->post(), Eq(true));
    EXPECT_THAT(semaphore->post(), Eq(true));
    EXPECT_THAT(semaphore->post(), Eq(true));

    int i;
    EXPECT_THAT(semaphore->getValue(i), Eq(true));
    EXPECT_THAT(i, Eq(45));
}

TEST_F(Semaphore_test, TimedWaitValidUnnamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create(12);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += 20000;

    EXPECT_THAT(semaphore->timedWait(&ts, false), Eq(true));

    int i;
    EXPECT_THAT(semaphore->getValue(i), Eq(true));
    EXPECT_THAT(i, Eq(11));
}

TEST_F(Semaphore_test, TimedWaitValidNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create("/fuuSa", S_IRUSR | S_IWUSR, 42);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += 20000;

    EXPECT_THAT(semaphore->timedWait(&ts, false), Eq(true));

    int i;
    EXPECT_THAT(semaphore->getValue(i), Eq(true));
    EXPECT_THAT(i, Eq(41));
}

TEST_F(Semaphore_test, TryWaitValidUnnamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create(12);

    EXPECT_THAT(semaphore->tryWait(), Eq(true));

    int i;
    EXPECT_THAT(semaphore->getValue(i), Eq(true));
    EXPECT_THAT(i, Eq(11));
}

TEST_F(Semaphore_test, TryWaitValidNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create("/fuuSa", S_IRUSR | S_IWUSR, 42);

    EXPECT_THAT(semaphore->tryWait(), Eq(true));

    int i;
    EXPECT_THAT(semaphore->getValue(i), Eq(true));
    EXPECT_THAT(i, Eq(41));
}

TEST_F(Semaphore_test, WaitValidUnnamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create(12);

    EXPECT_THAT(semaphore->wait(), Eq(true));

    int i;
    EXPECT_THAT(semaphore->getValue(i), Eq(true));
    EXPECT_THAT(i, Eq(11));
}

TEST_F(Semaphore_test, WaitValidNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create("/fuuSa", S_IRUSR | S_IWUSR, 42);

    EXPECT_THAT(semaphore->wait(), Eq(true));

    int i;
    EXPECT_THAT(semaphore->getValue(i), Eq(true));
    EXPECT_THAT(i, Eq(41));
}

TEST_F(Semaphore_test, MoveAssignmentNamed)
{
    iox::posix::Semaphore b;
    {
        auto semaphore = iox::posix::Semaphore::create("/fuuSa1", S_IRUSR | S_IWUSR, 42);
        b = std::move(*semaphore);
        EXPECT_THAT(semaphore->post(), Eq(false));
    }

    EXPECT_THAT(b.post(), Eq(true));
}

TEST_F(Semaphore_test, MoveAssignmentUnnamed)
{
    iox::posix::Semaphore b;
    {
        auto semaphore = iox::posix::Semaphore::create(12);
        b = std::move(*semaphore);
        EXPECT_THAT(semaphore->post(), Eq(false));
    }

    EXPECT_THAT(b.post(), Eq(true));
}

TEST_F(Semaphore_test, MoveCTorNamed)
{
    auto semaphore = iox::posix::Semaphore::create("/fuuSa1", S_IRUSR | S_IWUSR, 42);
    iox::posix::Semaphore b(std::move(*semaphore));

    EXPECT_THAT(b.post(), Eq(true));
    EXPECT_THAT(semaphore->post(), Eq(false));
}

TEST_F(Semaphore_test, MoveCTorUnnamed)
{
    auto semaphore = iox::posix::Semaphore::create(42);
    iox::posix::Semaphore b(std::move(*semaphore));

    EXPECT_THAT(b.post(), Eq(true));
    EXPECT_THAT(semaphore->post(), Eq(false));
}

#endif // not defined QNX
