// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_TOPIC_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_TOPIC_HPP

#include "iceoryx_hoofs/cxx/serialization.hpp"
#include "iceoryx_hoofs/cxx/type_traits.hpp"

#include <chrono>
#include <cstdint>

constexpr uint32_t ONE_KILOBYTE = 1000U;

struct AddRequest
{
    uint64_t addend1{0};
    uint64_t addend2{0};
};

struct AddResponse
{
    uint64_t sum{0};
};

struct Topic
{
    uint32_t counter{0};
};

template <uint32_t NumberOfBytes>
struct TimestampTopic
{
    // Printed to console
    uint32_t counter{0};
    std::chrono::time_point<std::chrono::steady_clock> sendTimestamp;

    // Not printed to console
    static constexpr uint32_t payloadSizeInBytes{NumberOfBytes};
    char data[NumberOfBytes];
    uint32_t subPackets{0};
};

using TimestampTopic1Byte = TimestampTopic<1>;
using TimestampTopic4Kb = TimestampTopic<4 * ONE_KILOBYTE>;
using TimestampTopic16Kb = TimestampTopic<16 * ONE_KILOBYTE>;
using TimestampTopic64Kb = TimestampTopic<64 * ONE_KILOBYTE>;
using TimestampTopic256Kb = TimestampTopic<256 * ONE_KILOBYTE>;
using TimestampTopic1Mb = TimestampTopic<1024 * ONE_KILOBYTE>;
using TimestampTopic4Mb = TimestampTopic<4096 * ONE_KILOBYTE>;

template <typename T, typename = void, typename = void, typename = void, typename = void, typename = void>
struct is_supported_topic : std::false_type
{
};

template <typename T>
struct is_supported_topic<T,
                          iox::cxx::void_t<decltype(T::counter)>,
                          iox::cxx::void_t<decltype(T::sendTimestamp)>,
                          iox::cxx::void_t<decltype(T::payloadSizeInBytes)>,
                          iox::cxx::void_t<decltype(T::data)>,
                          iox::cxx::void_t<decltype(T::subPackets)>> : std::true_type
{
};


#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_TOPIC_HPP
