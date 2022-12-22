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

#include "iceoryx_hoofs/cxx/convert.hpp"
#include "iceoryx_hoofs/internal/units/duration.hpp"

#include <atomic>
#include <chrono>
#include <iomanip>
#include <string>
#include <thread>

#if defined(__clang__)
std::string compiler =
    "clang-" + iox::cxx::convert::toString(__clang_major__) + "." + iox::cxx::convert::toString(__clang_minor__);
#elif defined(__GNUC__)
std::string compiler =
    "gcc-" + iox::cxx::convert::toString(__GNUC__) + "." + iox::cxx::convert::toString(__GNUC_MINOR__);
#elif defined(_MSC_VER)
std::string compiler = "msvc-" + iox::cxx::convert::toString(_MSC_VER);
#endif

#define BENCHMARK(f, duration) PerformBenchmark(f, #f, duration)

template <typename Return>
void PerformBenchmark(Return (&f)(), const char* functionName, const iox::units::Duration& duration)
{
    std::atomic_bool keepRunning{true};
    uint64_t numberOfCalls{0U};
    std::thread t([&] {
        while (keepRunning)
        {
            f();
            ++numberOfCalls;
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(duration.toMilliseconds()));
    keepRunning = false;
    t.join();

    // Not using iceoryx logger due to width requirements
    std::cout << std::setw(16) << compiler << " [ " << duration << " ] " << std::setw(15) << numberOfCalls << " : "
              << functionName << std::endl;
}
