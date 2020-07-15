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

#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

#include "benchmark.hpp"

uint64_t globalCounter{0U};

uint64_t simpleReturn()
{
    uint64_t returnValue = globalCounter + 1;
    return returnValue;
}

iox::cxx::optional<uint64_t> simpleReturnOptional()
{
    uint64_t returnValue = globalCounter + 1;
    return returnValue;
}

bool __popFromFiFo(uint64_t& value)
{
    ++globalCounter;
    uint64_t mod = globalCounter % 8;

    if (mod >= 4)
        return false;

    value = mod;
    return true;
}

void popFromFiFo()
{
    uint64_t maybeValue;
    if (__popFromFiFo(maybeValue))
        globalCounter += maybeValue;
    else
        --globalCounter;
}

iox::cxx::optional<uint64_t> __popFromFiFoOptional()
{
    ++globalCounter;
    uint64_t mod = globalCounter % 8;

    if (mod >= 4)
        return iox::cxx::nullopt;

    return mod;
}

void popFromFiFoOptional()
{
    __popFromFiFoOptional().and_then([](uint64_t value) { globalCounter += value; }).or_else([] { --globalCounter; });
}

uint64_t __complexErrorValue(uint64_t& value)
{
    ++globalCounter;
    uint64_t mod = globalCounter % 8;

    if (mod >= 4)
        return mod;

    value = mod;
    return 0;
}

void complexErrorValue()
{
    uint64_t maybeValue;
    uint64_t returnValue = __complexErrorValue(maybeValue);
    if (returnValue == 0)
        globalCounter += maybeValue;
    else
        globalCounter -= returnValue;
}

iox::cxx::expected<uint64_t, uint64_t> __complexErrorValueExpected()
{
    ++globalCounter;
    uint64_t mod = globalCounter % 8;

    if (mod >= 4)
        return iox::cxx::error<uint64_t>(mod);

    return iox::cxx::success<uint64_t>(mod);
}

void complexErrorValueExpected()
{
    __complexErrorValueExpected().and_then([](uint64_t value) { globalCounter += value; }).or_else([](uint64_t value) {
        globalCounter -= value;
    });
}

int main()
{
    using namespace iox::units::duration_literals;
    auto timeout = 1_s;

    BENCHMARK(simpleReturn, timeout);
    BENCHMARK(simpleReturnOptional, timeout);
    BENCHMARK(popFromFiFo, timeout);
    BENCHMARK(popFromFiFoOptional, timeout);
    BENCHMARK(complexErrorValue, timeout);
    BENCHMARK(complexErrorValueExpected, timeout);
}
