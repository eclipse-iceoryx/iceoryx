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

#ifndef IOX_UTILS_TESTUTILS_SEPPUKU_HPP
#define IOX_UTILS_TESTUTILS_SEPPUKU_HPP

#include <thread>

using namespace iox::units::duration_literals;

// class for killing the application if a test takes too much time to finish
class Seppuku
{
  public:
    Seppuku(const iox::units::Duration& timeToWait) noexcept
        : m_timeToWait(timeToWait)
    {
    }

    ~Seppuku() noexcept
    {
        m_seppukuSemaphore.post();
        m_seppuku.join();
    }

    void doSeppuku(std::function<void()> f) noexcept
    {
        struct timespec timeout = m_timeToWait.timespec(iox::units::TimeSpecReference::Epoch);
        m_seppuku = std::thread([=] {
            m_seppukuSemaphore.timedWait(&timeout, false)
                .and_then([&](auto& result) {
                    if (result == iox::posix::SemaphoreWaitState::TIMEOUT)
                    {
                        f();
                        EXPECT_TRUE(false);
                    }
                })
                .or_else([](auto&) { EXPECT_TRUE(false); });
        });
    }

  private:
    iox::units::Duration m_timeToWait{0_s};
    iox::posix::Semaphore m_seppukuSemaphore{
        iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U).value()};
    std::thread m_seppuku;
};

#endif // IOX_UTILS_TESTUTILS_SEPPUKU_HPP
