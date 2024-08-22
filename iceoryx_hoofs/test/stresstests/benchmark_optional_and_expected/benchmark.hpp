// Copyright (c) 2020 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2022 Apex.AI Inc. All rights reserved.
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
#include "iox/duration.hpp"

#include <chrono>
#include <iomanip>
#include <string>
#include <thread>

#if defined(__clang__)
std::string compiler = "clang-" + std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__);
#elif defined(__GNUC__)
std::string compiler = "gcc-" + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__);
#elif defined(_MSC_VER)
std::string compiler = "msvc-" + std::to_string(_MSC_VER);
#endif

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage) Not all of the functionality of this macro can be achieved by a constexpr template
#define BENCHMARK(f, duration) PerformBenchmark(f, #f, duration)

template <typename Return>
void PerformBenchmark(Return (&f)(), const char* functionName, const iox::units::Duration& duration)
{
    iox::concurrent::Atomic<bool> keepRunning{true};
    uint64_t numberOfCalls{0U};
    uint64_t actualDurationNanoSeconds{0};
    std::thread t([&] {
        auto start = std::chrono::system_clock::now();
        while (keepRunning)
        {
            f();
            ++numberOfCalls;
        }
        auto end = std::chrono::system_clock::now();
        auto actualDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        actualDurationNanoSeconds = static_cast<uint64_t>(actualDuration.count());
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(duration.toMilliseconds()));
    keepRunning = false;
    t.join();

    // Not using iceoryx logger due to width requirements
    auto seconds = actualDurationNanoSeconds / iox::units::Duration::NANOSECS_PER_SEC;
    auto nanosecs = actualDurationNanoSeconds % iox::units::Duration::NANOSECS_PER_SEC;
    std::cout << std::setw(16) << compiler << " [ " << std::setw(1) << seconds << "s " << std::setw(9) << nanosecs
              << "ns ] " << std::setw(15) << numberOfCalls << " (iters) : " << std::setw(6)
              << actualDurationNanoSeconds / numberOfCalls << " (nanosecs/iters) : " << functionName << std::endl;
}
