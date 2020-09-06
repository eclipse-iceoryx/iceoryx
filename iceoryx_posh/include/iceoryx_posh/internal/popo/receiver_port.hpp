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
#ifndef IOX_POSH_POPO_RECEIVER_PORT_HPP
#define IOX_POSH_POPO_RECEIVER_PORT_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/chunk_management.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/popo/ports/base_port.hpp"
#include "iceoryx_posh/internal/popo/receiver_port_data.hpp"
#include "iceoryx_posh/internal/popo/used_chunk_list.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/internal/concurrent/sofi.hpp"
#include "iceoryx_utils/internal/posix_wrapper/mutex.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"


#include <cstdint>

namespace iox
{
namespace popo
{
class ReceiverPort : public BasePort
{
  public:
    using MemberType_t = ReceiverPortData;
    using mutex_t = posix::mutex;
    using MemoryInfo = iox::mepoo::MemoryInfo;

    // BEGIN REGION__ROUDI // /* access from RouDi------------------------------------

    ReceiverPort(ReceiverPortData* f_member);
    ReceiverPort(const ReceiverPort& other) = delete;
    ReceiverPort& operator=(const ReceiverPort& other) = delete;

    ReceiverPort(ReceiverPort&& rhs) = default;
    ReceiverPort& operator=(ReceiverPort&& rhs) = default;

    cxx::optional<capro::CaproMessage> getCaProMessage();

    cxx::optional<capro::CaproMessage> dispatchCaProMessage(capro::CaproMessage f_caProMessage);

    virtual void cleanup() noexcept;

    // END REGION__ROUDI

    // BEGIN REGION__APPLICATION // /* access from Application-------------------------------

    virtual void subscribe(const bool f_autoResubscribe = false,
                           const uint32_t f_deliverySize = MAX_SUBSCRIBER_QUEUE_CAPACITY); // deprecated
    virtual void subscribe(const uint32_t f_deliverySize = MAX_SUBSCRIBER_QUEUE_CAPACITY);
    void unsubscribe();
    bool isSubscribed() const;
    SubscribeState getSubscribeState() const;

    // (only) from delivery FiFo to Cache
    virtual bool getChunk(const mepoo::ChunkHeader*& f_chunkHeader) noexcept;
    bool releaseChunk(const mepoo::ChunkHeader* f_chunkHeader);

    bool getSharedChunk(mepoo::SharedChunk& f_chunk);
    virtual bool newData() noexcept;
    void clearDeliveryFiFo();

    /* expects an initialized POSIX semaphore, stored in shared memory! */
    virtual void SetCallbackReferences(posix::Semaphore* f_callbackSemaphore) noexcept;
    virtual void UnsetCallbackReferences() noexcept;

    bool AreCallbackReferencesSet();

    // offer a 'local' semaphore, stored in shared memory, that can be used with
    // 'SetReceiveSemaphore(sem_t*)'
    posix::Semaphore* GetShmSemaphore();

    virtual bool deliver(mepoo::SharedChunk f_chunk_p) noexcept;

    bool isInternal() const;

    uint64_t getDeliveryFiFoCapacity() const;
    uint64_t getDeliveryFiFoSize() const;

    /// @deprecated function for enabling/disabling notifications when the deliveryfifo is dropping messages (e.g. SoFi
    /// is used) Enables a bool which is evaluated in delivery() function for counting dropped samples and printing
    /// logmessages.
    /// @todo integrate that function cleaner with next refactoring
    void setNotifyOnOverflow(const bool) noexcept;

    const MemoryInfo& getMemoryInfo() const noexcept;

  private:
    const MemberType_t* getMembers() const noexcept;
    MemberType_t* getMembers() noexcept;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/receiver_port.inl"

#endif // IOX_POSH_POPO_RECEIVER_PORT_HPP
