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

#pragma once

#include "test.hpp"

#include "iceoryx_posh/internal/popo/sender_port.hpp"

#include "iceoryx_posh/mepoo/chunk_header.hpp"

#include <cstdint>
#include <memory>

class SenderPort_MOCK
{
  public:
    struct mock_t
    {
        uint64_t activate{0};
        uint64_t deactivate{0};
        uint64_t hasSubscribers{0};
        bool hasSubscribersReturn{false};
        uint64_t enableDoDeliverOnSubscription{0};
        uint64_t reserveChunk{0};
        uint64_t deliverChunk{0};
        iox::mepoo::ChunkHeader* reserveSampleReturn{nullptr};
        uint64_t doesDeliverOnSubscribe{0};
        bool doesDeliverOnSubscribeReturn{false};
        uint64_t getUniqueID{0};
        uint64_t getUniqueIDReturn{0};
        uint64_t getThroughput{0};
        typename iox::popo::SenderPortData::Throughput getThroughputReturn;
        uint64_t getNanosecondsBetweenLastTwoDeliveries{0};
        uint64_t getNanosecondsBetweenLastTwoDeliveriesReturn{0};
        uint64_t isConnectedToMembers;
        bool isConnectedToMembersReturn{false};
    };
    mutable std::shared_ptr<mock_t> details{new mock_t()};
    static std::shared_ptr<mock_t> globalDetails;

    SenderPort_MOCK() = default;
    SenderPort_MOCK(iox::popo::SenderPortData* const)
    {
    }

    SenderPort_MOCK(SenderPort_MOCK&&) = default;
    SenderPort_MOCK& operator=(SenderPort_MOCK&&) = default;
    SenderPort_MOCK(const SenderPort_MOCK&) = default;
    SenderPort_MOCK& operator=(const SenderPort_MOCK&) = default;

    using Throughput = iox::popo::SenderPortData::Throughput;
    using MemberType_t = iox::popo::SenderPortData;
    void activate()
    {
        if (globalDetails)
        {
            globalDetails->activate++;
        }
        details->activate++;
    }
    void deactivate()
    {
        if (globalDetails)
        {
            globalDetails->deactivate++;
        }
        details->deactivate++;
    }
    bool hasSubscribers()
    {
        details->hasSubscribers++;
        if (globalDetails)
        {
            globalDetails->hasSubscribers++;
            return globalDetails->hasSubscribersReturn;
        }
        return details->hasSubscribersReturn;
    }
    void enableDoDeliverOnSubscription()
    {
        if (globalDetails)
        {
            globalDetails->enableDoDeliverOnSubscription++;
        }
        details->enableDoDeliverOnSubscription++;
    }
    iox::mepoo::ChunkHeader* reserveChunk(uint32_t)
    {
        details->reserveChunk++;
        if (globalDetails)
        {
            globalDetails->reserveChunk++;
            return globalDetails->reserveSampleReturn;
        }
        return details->reserveSampleReturn;
    }
    void deliverChunk(iox::mepoo::ChunkHeader* const)
    {
        if (globalDetails)
        {
            globalDetails->deliverChunk++;
        }
        details->deliverChunk++;
    }
    bool doesDeliverOnSubscribe() const
    {
        details->doesDeliverOnSubscribe++;
        if (globalDetails)
        {
            globalDetails->doesDeliverOnSubscribe++;
            return globalDetails->doesDeliverOnSubscribeReturn;
        }
        return details->doesDeliverOnSubscribeReturn;
    }
    uint64_t getUniqueID() const
    {
        details->getUniqueID++;
        if (globalDetails)
        {
            globalDetails->getUniqueID++;
            return globalDetails->getUniqueIDReturn;
        }
        return details->getUniqueIDReturn;
    }
    typename iox::popo::SenderPortData::Throughput getThroughput() const
    {
        details->getThroughput++;
        if (globalDetails)
        {
            globalDetails->getThroughput++;
            return globalDetails->getThroughputReturn;
        }
        return details->getThroughputReturn;
    }
    uint64_t getNanosecondsBetweenLastTwoDeliveries()
    {
        details->getNanosecondsBetweenLastTwoDeliveries++;
        if (globalDetails)
        {
            globalDetails->getNanosecondsBetweenLastTwoDeliveries++;
            return globalDetails->getNanosecondsBetweenLastTwoDeliveriesReturn;
        }
        return details->getNanosecondsBetweenLastTwoDeliveriesReturn;
    }
    operator bool() const
    {
        details->isConnectedToMembers++;
        if (globalDetails)
        {
            globalDetails->isConnectedToMembers++;
            return globalDetails->isConnectedToMembersReturn;
        }
        return details->isConnectedToMembersReturn;
    }
};
