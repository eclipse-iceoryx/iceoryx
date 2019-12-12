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

#include "iceoryx_posh/popo/subscriber.hpp"

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/internal/posix_wrapper/timespec.hpp"

#include "ac3log/simplelogger.hpp"

#include <cassert>

namespace iox
{
namespace popo
{
Subscriber::Subscriber() noexcept
{
}

Subscriber::Subscriber(const capro::ServiceDescription& service, const cxx::CString100& runnableName) noexcept
    : m_receiver(
        runtime::PoshRuntime::getInstance().getMiddlewareReceiver(service, Interfaces::INTERNAL, runnableName))
{
}

Subscriber::~Subscriber() noexcept
{
    unsetReceiveHandler();
    // TODO: Find an alternative like an RAII receive handler which
    //          is called in the dtor. You cannot expect the user to call it before destruction
}

void Subscriber::subscribe(const uint32_t cacheSize) noexcept
{
    m_subscribeDemand = true;
    uint32_t size = cacheSize;
    if (size > MAX_RECEIVER_QUEUE_SIZE)
    {
        WARN_PRINTF("Cache size for subscribe too large %lu, limiting to MAX_RECEIVER_QUEUE_SIZE = %lu\n",
                    size,
                    MAX_RECEIVER_QUEUE_SIZE);
        size = MAX_RECEIVER_QUEUE_SIZE;
    }
    m_receiver.subscribe(true, size);
}

SubscriptionState Subscriber::getSubscriptionState() const noexcept
{
    if (!m_subscribeDemand)
    {
        return SubscriptionState::NOT_SUBSCRIBED;
    }
    else
    {
        if (m_receiver.isSubscribed())
        {
            return SubscriptionState::SUBSCRIBED;
        }
        else
        {
            return SubscriptionState::SUBSCRIPTION_PENDING;
        }
    }
}

void Subscriber::unsubscribe() noexcept
{
    m_receiver.unsubscribe();
    m_subscribeDemand = false;
}

void Subscriber::setReceiveHandler(ReceiveHandler_t cbHandler) noexcept
{
    // no need to guard this assignment since the thread accessing the cb
    // handler will be created later
    assert(m_callbackHandler == nullptr && "SetReceiveHandler: callback handler already set - please Unset first.");
    m_callbackHandler = cbHandler;

    // (re-)init thread & semaphore
    m_callbackSemaphore = m_receiver.GetShmSemaphore();
    if (!m_callbackSemaphore)
    {
        m_callbackHandler = nullptr;
        WARN_PRINTF("shared memory semaphore could not be initialized!\n");
        return;
    }
    m_receiver.SetCallbackReferences(m_callbackSemaphore);

    m_callbackRunFlag = true;
    m_callbackThread = std::thread(&Subscriber::eventCallbackMain, this);
// name thread if possible
#ifdef __unix__
    char threadname[16];
    static ushort thread_index = 0;
    snprintf(threadname, sizeof(threadname), "receiver-cb_%d", ++thread_index);
    auto thread_handle = m_callbackThread.native_handle();
    pthread_setname_np(thread_handle, threadname); // thread name restricted to 16 chars
// (incl. '0') but name buffer needs to be at least 16 chars
#endif // __unix__
    m_callbackThreadPresent = true;
}

void Subscriber::unsetReceiveHandler() noexcept
{
    // stop callback thread
    m_callbackRunFlag = false;

    if (m_callbackSemaphore)
    {
        m_callbackSemaphore->post();
        m_receiver.UnsetCallbackReferences();
    }

    if (m_callbackThread.joinable())
    {
        m_callbackThread.join();
    }

    // no need to guard this assignment since the thread accessing the cb
    // handler has been joined already
    m_callbackHandler = nullptr;
    m_callbackThreadPresent = false;
}

void Subscriber::overrideCallbackReference(const Subscriber& receiverWithRererenceToUse) noexcept
{
    const auto semaphore = receiverWithRererenceToUse.m_receiver.GetShmSemaphore();
    assert(semaphore != nullptr && "OverrideCallbackReference: source semaphore is not set");
    m_receiver.SetCallbackReferences(semaphore);
}

bool Subscriber::waitForChunk(const uint32_t timeoutMs) noexcept
{
    const auto semaphore = m_receiver.GetShmSemaphore();
    assert(semaphore != nullptr && "WaitForChunk: semaphore is not set");

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts = posix::addTimeMs(ts, timeoutMs);
    return semaphore->timedWait(&ts, true);
}

bool Subscriber::tryWaitForChunk() noexcept
{
    const auto semaphore = m_receiver.GetShmSemaphore();
    assert(semaphore != nullptr && "TryWaitForChunk: semaphore is not set");

    return semaphore->tryWait();
}

bool Subscriber::getChunk(const mepoo::ChunkHeader** chunkHeader) noexcept
{
    return (m_receiver.getChunk(*chunkHeader));
}

bool Subscriber::getChunk(const void** payload) noexcept
{
    const mepoo::ChunkHeader* chunkHeader = nullptr;
    if (m_receiver.getChunk(chunkHeader))
    {
        *payload = chunkHeader->payload();
        return true;
    }
    else
    {
        *payload = nullptr;
        return false;
    }
}

void Subscriber::deleteNewChunks() noexcept
{
    m_receiver.clearDeliveryFiFo();
}

bool Subscriber::releaseChunk(const mepoo::ChunkHeader* const chunkHeader) noexcept
{
    return m_receiver.releaseChunk(chunkHeader);
}

bool Subscriber::releaseChunk(const void* const payload) noexcept
{
    auto chunkHeader = iox::mepoo::convertPayloadPointerToChunkHeader(const_cast<void* const>(payload));
    return m_receiver.releaseChunk(chunkHeader);
}

bool Subscriber::hasNewChunks() const noexcept
{
    return m_receiver.newData();
}

posix::Semaphore* Subscriber::getSemaphore() const noexcept
{
    // Temporary solution as long as there is no other mechanism to request a semophore
    auto semaphore = m_receiver.GetShmSemaphore();
    assert(semaphore != nullptr && "GetSemaphore: semaphore is not set");

    return semaphore;
}

void Subscriber::setChunkReceiveSemaphore(posix::Semaphore* semaphore) noexcept
{
    m_receiver.SetCallbackReferences(semaphore);
}

bool Subscriber::isChunkReceiveSemaphoreSet() noexcept
{
    return m_receiver.AreCallbackReferencesSet();
}

void Subscriber::eventCallbackMain() noexcept
{
    while (m_callbackRunFlag)
    {
        if (m_callbackSemaphore && !m_callbackSemaphore->wait())
        {
            // TODO: error handling
        }
        // in case new data arrived during a call to the handler, we
        // might loop the outer while-loop several times without
        // actually calling the handler, just to decrease the semaphore
        // since the latest data items were already fetched by call(s)
        // to 'GetChunk' [alias 'Update' on event level]
        if (m_receiver.newData() && m_callbackRunFlag)
        {
            std::lock_guard<mutex_t> g(m_callbackHandlerMutex);

            m_callbackHandler();
        }
    }
}

} // namespace popo
} // namespace iox
