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

#include "iceoryx_posh/internal/popo/sender_port_data.hpp"

#include <cstdint>

namespace iox
{
namespace popo
{
class SenderPort : public BasePort
{
  public:
    using MemberType_t = SenderPortData;

    SenderPort(SenderPortData* const member);

    SenderPort(const SenderPort& other) = delete;
    SenderPort& operator=(const SenderPort&) = delete;
    SenderPort(SenderPort&& rhs) = default;
    SenderPort& operator=(SenderPort&& rhs) = default;
    ~SenderPort() = default;

    cxx::optional<capro::CaproMessage> getCaProMessage();

    cxx::optional<capro::CaproMessage> dispatchCaProMessage(capro::CaproMessage caProMessage);

    void cleanup();
    mepoo::ChunkHeader* reserveChunk(const uint32_t payloadSize, bool useDynamicPayloadSizes = false);
    virtual void deliverChunk(mepoo::ChunkHeader* const chunkHeader);
    void freeChunk(mepoo::ChunkHeader* const chunkHeader);
    void activate();
    void deactivate();
    bool hasSubscribers();
    void forwardChunk(mepoo::SharedChunk chunk);
    MemberType_t::Throughput getThroughput() const;
    void enableDoDeliverOnSubscription();
    bool doesDeliverOnSubscribe() const;
    bool isPortActive() const;
    uint32_t getMaxDeliveryFiFoCapacity();

  protected:
    virtual bool connectReceiverPort(ReceiverPortType::MemberType_t* const receiver);

  private:
    bool hasValidService(const capro::CaproMessage& caproMessage);
    void disconnectAllReceiver();
    void setThroughput(const uint32_t payloadSize);
    void setThroughputDeliveryData(mepoo::ChunkInfo& chunk, bool updateTimeInChunk = true);

    void disconnectReceiverPort(ReceiverPortType::MemberType_t* const receiver);

    bool pushToAllocatedChunkContainer(mepoo::SharedChunk chunk);
    bool popFromAllocatedChunkContainer(mepoo::ChunkHeader* chunkHeader, mepoo::SharedChunk& chunk);
    bool deleteFromAllocatedChunkContainer(mepoo::ChunkHeader* chunkHeader);
    void clearAllocatedChunkContainer();

    const MemberType_t* getMembers() const;
    MemberType_t* getMembers();
};

} // namespace popo
} // namespace iox
