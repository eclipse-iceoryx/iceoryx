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

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/fixed_string/string100.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"

#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

namespace iox
{
namespace popo
{
enum class SubscriptionState
{
    SUBSCRIBED,
    NOT_SUBSCRIBED,
    SUBSCRIPTION_PENDING
};

class Subscriber
{
  public:
    using mutex_t = std::mutex;
    using ReceiveHandler_t = std::function<void(void)>;

    /// @brief Constructor
    /// @param[in] service Information on service , service, instance, event Id
    /// @param[in] runnableName optional name of the runnable the subscriber belongs to
    explicit Subscriber(const capro::ServiceDescription& service, const cxx::CString100& runnableName = "") noexcept;

    /// @brief Destructor for event receiver
    virtual ~Subscriber() noexcept;

    Subscriber& operator=(const Subscriber& other) = delete;
    Subscriber(const Subscriber& other) = delete;

    Subscriber(Subscriber&& other) = default;
    Subscriber& operator=(Subscriber&&) = default;

    /// @brief Function for subscribing to event
    /// @param[in] cacheSize Size of the receiver queue
    void subscribe(const uint32_t cacheSize = MAX_RECEIVER_QUEUE_SIZE) noexcept;

    /// @brief Get function for retrieving subsription state
    /// @return enum value of subsription state
    SubscriptionState getSubscriptionState() const noexcept;

    /// @brief Function for Unsubscribing to an event
    void unsubscribe() noexcept;

    /// @brief Set function for Receiver callback handler
    /// @param[in] handler pointer to the receiver
    void setReceiveHandler(ReceiveHandler_t handler) noexcept;

    /// @brief unset the value of callback handler
    void unsetReceiveHandler() noexcept;

    /// @brief set the callback reference with shared memory semaphore
    /// @param[in] receiverWithRererenceToUse to get the shared memory semaphore
    void overrideCallbackReference(const Subscriber& receiverWithRererenceToUse) noexcept;

    /// @brief Function for timed wait to receive a chunk
    /// @param[in] timeoutMs the time in Milliseconds to wait
    /// @return true when restart is needed on interruption by handler, false when timed out
    bool waitForChunk(const uint32_t timeoutMs) noexcept;

    /// @brief Function for try wait to receive a chunk
    /// @param[in] timeoutMs the time in Milliseconds to wait
    /// @return true on success otherwise false
    bool tryWaitForChunk() noexcept;

    /// @brief Get Function for chunk
    /// @param[in,out] chunkHeader Information of the chunk received.
    /// @return true when the chunk is received otherwise false
    bool getChunk(const mepoo::ChunkHeader** chunkHeader) noexcept;

    /// @brief Get Function for chunk
    /// @param[in,out] payload pointer to the chunk payload.
    /// @return true when the chunk is received otherwise false
    bool getChunk(const void** payload) noexcept;

    /// @brief Function for cleaning up the stored smaples in FIFO
    void deleteNewChunks() noexcept;

    /// @brief Function for releasing particular chunk
    /// @param[in] chunkHeader Information of the chunk to be released.
    /// @return true when the chunk is deleted , false when pointer is invalid
    bool releaseChunk(const mepoo::ChunkHeader* const chunkHeader) noexcept;

    /// @brief Function for releasing particular chunk
    /// @param[in] payload pointer to the payload of the chunk to be released.
    /// @return true when the chunk is deleted , false when pointer is invalid
    bool releaseChunk(const void* const payload) noexcept;

    /// @brief Function to check the availability of new chunk
    /// @return true when new chunk is available otherwise false
    bool hasNewChunks() const noexcept;

    /// @brief Get function for shared memory semaphore
    /// @return  posix::Semaphore pointer to shared memory
    posix::Semaphore* getSemaphore() const noexcept;

    /// @brief Set function for callback reference assignation with semaphore
    /// @param[in] semaphore pointer to shared memory
    void setChunkReceiveSemaphore(posix::Semaphore* semaphore) noexcept;

    /// @brief check function to confirm whether callback references are set
    /// @return true if the references are set otherwise false
    bool isChunkReceiveSemaphoreSet() noexcept;

  protected:
    // needed for unit testing
    Subscriber() noexcept;

  private:
    // callback main method
    void eventCallbackMain() noexcept;

    mutable ReceiverPortType m_receiver{
        nullptr}; /// @todo remove mutable, required since the receiverPort is modifying data in const methods
    bool m_subscribeDemand{false};

    // event callback related
    mutable mutex_t m_callbackHandlerMutex;
    ReceiveHandler_t m_callbackHandler{nullptr};

    std::thread m_callbackThread;
    std::atomic_bool m_callbackThreadPresent{false};
    std::atomic_bool m_callbackRunFlag{false};
    posix::Semaphore* m_callbackSemaphore{nullptr};
};

} // namespace popo
} // namespace iox
