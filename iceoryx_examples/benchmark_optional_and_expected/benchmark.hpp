// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/internal/units/duration.hpp"

#include <atomic>
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

    std::this_thread::sleep_for(std::chrono::milliseconds(duration.milliSeconds<uint64_t>()));
    keepRunning = false;
    t.join();

    std::cout << std::setw(16) << compiler << " [ " << duration << " ] " << std::setw(15) << numberOfCalls << " : "
              << functionName << std::endl;
}
