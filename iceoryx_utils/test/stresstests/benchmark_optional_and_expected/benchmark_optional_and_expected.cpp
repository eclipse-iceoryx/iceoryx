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

template <uint64_t Size>
struct LargeObject
{
    uint64_t value;
    char dataBlob[Size];
};

template <uint64_t Size>
struct LargeObjectComplexCTor
{
    LargeObjectComplexCTor()
    {
        for (uint64_t i = 0u; i < Size; ++i)
        {
            dataBlob[i] = static_cast<char>((++globalCounter) % 256);
        }
    };
    uint64_t value;
    char dataBlob[Size];
};

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

bool popFromFiFoImpl(uint64_t& value)
{
    ++globalCounter;
    uint64_t mod = globalCounter % 8;

    if (mod >= 4)
    {
        return false;
    }

    value = mod;
    return true;
}

void popFromFiFo()
{
    uint64_t maybeValue;
    if (popFromFiFoImpl(maybeValue))
    {
        globalCounter += maybeValue;
    }
    else
    {
        --globalCounter;
    }
}

iox::cxx::optional<uint64_t> popFromFiFoOptionalImpl()
{
    ++globalCounter;
    uint64_t mod = globalCounter % 8;

    if (mod >= 4)
    {
        return iox::cxx::nullopt;
    }

    return mod;
}

void popFromFiFoOptional()
{
    popFromFiFoOptionalImpl().and_then([](uint64_t value) { globalCounter += value; }).or_else([] { --globalCounter; });
}

uint64_t complexErrorValueImpl(uint64_t& value)
{
    ++globalCounter;
    uint64_t mod = globalCounter % 8;

    if (mod >= 4)
    {
        return mod;
    }

    value = mod;
    return 0;
}

void complexErrorValue()
{
    uint64_t maybeValue;
    uint64_t returnValue = complexErrorValueImpl(maybeValue);
    if (returnValue == 0)
    {
        globalCounter += maybeValue;
    }
    else
    {
        globalCounter -= returnValue;
    }
}

iox::cxx::expected<uint64_t, uint64_t> complexErrorValueExpectedImpl()
{
    ++globalCounter;
    uint64_t mod = globalCounter % 8;

    if (mod >= 4)
    {
        return iox::cxx::error<uint64_t>(mod);
    }

    return iox::cxx::success<uint64_t>(mod);
}

void complexErrorValueExpected()
{
    complexErrorValueExpectedImpl()
        .and_then([](uint64_t value) { globalCounter += value; })
        .or_else([](uint64_t value) { globalCounter -= value; });
}

template <typename T>
bool largeObjectPopPlainImpl(T& value)
{
    ++globalCounter;

    if (globalCounter % 3 == 0)
    {
        return false;
    }

    T returnValue;
    returnValue.value = globalCounter;
    value = returnValue;

    return true;
}

template <typename T>
iox::cxx::optional<T> largeObjectPopOptionalImpl()
{
    ++globalCounter;

    if (globalCounter % 3 == 0)
    {
        return iox::cxx::nullopt;
    }

    T returnValue;
    returnValue.value = globalCounter;

    return returnValue;
}

template <typename T>
iox::cxx::expected<T, uint64_t> largeObjectPopExpectedImpl()
{
    ++globalCounter;

    if (globalCounter % 3 == 0)
    {
        return iox::cxx::error<uint64_t>(globalCounter);
    }

    T returnValue;
    returnValue.value = globalCounter;

    return iox::cxx::success<T>(returnValue);
}

template <typename T>
void largeObjectPopPlain()
{
    T value;
    if (largeObjectPopPlainImpl(value))
    {
        globalCounter += value.value;
    }
    else
    {
        --globalCounter;
    }
}

template <typename T>
void largeObjectPopOptional()
{
    largeObjectPopOptionalImpl<T>().and_then([](T& value) { globalCounter += value.value; }).or_else([] {
        --globalCounter;
    });
}

template <typename T>
void largeObjectPopExpected()
{
    largeObjectPopExpectedImpl<T>().and_then([](T& value) { globalCounter += value.value; }).or_else([](uint64_t) {
        --globalCounter;
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

    constexpr uint64_t LargeObjectSize = 1024;
    BENCHMARK(largeObjectPopPlain<LargeObject<LargeObjectSize>>, timeout);
    BENCHMARK(largeObjectPopOptional<LargeObject<LargeObjectSize>>, timeout);
    BENCHMARK(largeObjectPopExpected<LargeObject<LargeObjectSize>>, timeout);

    BENCHMARK(largeObjectPopPlain<LargeObjectComplexCTor<LargeObjectSize>>, timeout);
    BENCHMARK(largeObjectPopOptional<LargeObjectComplexCTor<LargeObjectSize>>, timeout);
    BENCHMARK(largeObjectPopExpected<LargeObjectComplexCTor<LargeObjectSize>>, timeout);
}
