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

struct TimestampTopic1Kb
{
    // Printed to console
    uint32_t counter{0};
    std::chrono::time_point<std::chrono::steady_clock> sendTimestamp;

    // Not printed to console
    static constexpr uint32_t payloadSizeInBytes{1};
    char data[payloadSizeInBytes];
    uint32_t subPackets{0};

    /// @todo #1332 Use cxx::Serialization and implement serialization for std::chrono::time_point?
    // iox::cxx::Serialization serialize() const noexcept
    // {
    //     return iox::cxx::Serialization::create(counter, sendTimestamp, payloadSizeInBytes, data, subPackets);
    // }
    // static iox::cxx::expected<TimestampTopic, iox::cxx::Serialization::Error>
    // deserialize(const iox::cxx::Serialization& serialized) noexcept
    // {
    //     TimestampTopic topic;

    //     auto deserializationSuccessful = serialized.extract(
    //         topic.counter, topic.sendTimestamp, topic.payloadSizeInBytes, topic.data, topic.subPackets);

    //     if (!deserializationSuccessful)
    //     {
    //         return iox::cxx::error<iox::cxx::Serialization::Error>(
    //             iox::cxx::Serialization::Error::DESERIALIZATION_FAILED);
    //     }

    //     return iox::cxx::success<TimestampTopic>(topic);
    // }
};

struct TimestampTopic4Kb
{
    // Printed to console
    uint32_t counter{0};
    std::chrono::time_point<std::chrono::steady_clock> sendTimestamp;

    // Not printed to console
    static constexpr uint32_t payloadSizeInBytes{4 * ONE_KILOBYTE};
    char data[payloadSizeInBytes];
    uint32_t subPackets{0};
};

struct TimestampTopic16Kb
{
    // Printed to console
    uint32_t counter{0};
    std::chrono::time_point<std::chrono::steady_clock> sendTimestamp;

    // Not printed to console
    static constexpr uint32_t payloadSizeInBytes{16 * ONE_KILOBYTE};
    char data[payloadSizeInBytes];
    uint32_t subPackets{0};
};

struct TimestampTopic64Kb
{
    // Printed to console
    uint32_t counter{0};
    std::chrono::time_point<std::chrono::steady_clock> sendTimestamp;

    // Not printed to console
    static constexpr uint32_t payloadSizeInBytes{64 * ONE_KILOBYTE};
    char data[payloadSizeInBytes];
    uint32_t subPackets{0};
};

struct TimestampTopic256Kb
{
    // Printed to console
    uint32_t counter{0};
    std::chrono::time_point<std::chrono::steady_clock> sendTimestamp;

    // Not printed to console
    static constexpr uint32_t payloadSizeInBytes{256 * ONE_KILOBYTE};
    char data[payloadSizeInBytes];
    uint32_t subPackets{0};
};

struct TimestampTopic1Mb
{
    // Printed to console
    uint32_t counter{0};
    std::chrono::time_point<std::chrono::steady_clock> sendTimestamp;

    // Not printed to console
    static constexpr uint32_t payloadSizeInBytes{1024 * ONE_KILOBYTE};
    char data[payloadSizeInBytes];
    uint32_t subPackets{0};
};

struct TimestampTopic4Mb
{
    // Printed to console
    uint32_t counter{0};
    std::chrono::time_point<std::chrono::steady_clock> sendTimestamp;

    // Not printed to console
    static constexpr uint32_t payloadSizeInBytes{4096 * ONE_KILOBYTE};
    char data[payloadSizeInBytes];
    uint32_t subPackets{0};
};


#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_TOPIC_HPP
