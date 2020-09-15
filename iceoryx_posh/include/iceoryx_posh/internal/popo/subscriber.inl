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
#ifndef IOX_POSH_POPO_SUBSCRIBER_INL
#define IOX_POSH_POPO_SUBSCRIBER_INL

namespace iox
{
namespace popo
{
template <typename ReceiverPortType>
inline Subscriber_t<ReceiverPortType>::Subscriber_t() noexcept
{
}

template <typename ReceiverPortType>
inline Subscriber_t<ReceiverPortType>::Subscriber_t(const capro::ServiceDescription& service,
                                                    const cxx::CString100& runnableName) noexcept
    : m_serviceDescription(service)
    , m_receiver(runtime::PoshRuntime::getInstance().getMiddlewareReceiver(service, runnableName))
{
}

template <typename ReceiverPortType>
inline Subscriber_t<ReceiverPortType>::~Subscriber_t() noexcept
{
    unsetReceiveHandler();
    // TODO: Find an alternative like an RAII receive handler which
    //          is called in the dtor. You cannot expect the user to call it before destruction
    if (m_receiver)
    {
        m_receiver.destroy();
    }
}

template <typename ReceiverPortType>
inline void Subscriber_t<ReceiverPortType>::subscribe(const uint32_t cacheSize) noexcept
{
    m_subscribeDemand = true;
    uint32_t size = cacheSize;
    if (size > MAX_SUBSCRIBER_QUEUE_CAPACITY)
    {
        LogWarn() << "Cache size for subscribe too large " << size
                  << ", limiting to MAX_SUBSCRIBER_QUEUE_CAPACITY = " << MAX_SUBSCRIBER_QUEUE_CAPACITY;
        size = MAX_SUBSCRIBER_QUEUE_CAPACITY;
    }
    m_receiver.subscribe(true, size);
}

template <typename ReceiverPortType>
inline SubscriptionState Subscriber_t<ReceiverPortType>::getSubscriptionState() const noexcept
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

template <typename ReceiverPortType>
inline void Subscriber_t<ReceiverPortType>::unsubscribe() noexcept
{
    m_receiver.unsubscribe();
    m_subscribeDemand = false;
}

template <typename ReceiverPortType>
inline void Subscriber_t<ReceiverPortType>::setReceiveHandler(ReceiveHandler_t cbHandler) noexcept
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
        LogWarn() << "Shared memory semaphore could not be initialized!";
        return;
    }
    m_receiver.SetCallbackReferences(m_callbackSemaphore);

    m_callbackRunFlag = true;
    m_callbackThread = std::thread(&Subscriber_t::eventCallbackMain, this);
// name thread if possible
#ifdef __unix__
    char threadname[16];
    static uint16_t thread_index = 0u;
    snprintf(threadname, sizeof(threadname), "Receive_%d", ++thread_index);
    auto thread_handle = m_callbackThread.native_handle();
    pthread_setname_np(thread_handle, threadname); // thread name restricted to 16 chars
// (incl. '0') but name buffer needs to be at least 16 chars
#endif // __unix__
    m_callbackThreadPresent = true;
}

template <typename ReceiverPortType>
inline void Subscriber_t<ReceiverPortType>::unsetReceiveHandler() noexcept
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

template <typename ReceiverPortType>
inline void
Subscriber_t<ReceiverPortType>::overrideCallbackReference(const Subscriber_t& receiverWithRererenceToUse) noexcept
{
    const auto semaphore = receiverWithRererenceToUse.m_receiver.GetShmSemaphore();
    assert(semaphore != nullptr && "OverrideCallbackReference: source semaphore is not set");
    m_receiver.SetCallbackReferences(semaphore);
}

template <typename ReceiverPortType>
inline bool Subscriber_t<ReceiverPortType>::waitForChunk(const uint32_t timeoutMs) noexcept
{
    const auto semaphore = m_receiver.GetShmSemaphore();
    assert(semaphore != nullptr && "WaitForChunk: semaphore is not set");

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts = posix::addTimeMs(ts, timeoutMs);
    return semaphore->timedWait(&ts, true);
}

template <typename ReceiverPortType>
inline bool Subscriber_t<ReceiverPortType>::tryWaitForChunk() noexcept
{
    const auto semaphore = m_receiver.GetShmSemaphore();
    assert(semaphore != nullptr && "TryWaitForChunk: semaphore is not set");

    return semaphore->tryWait();
}

template <typename ReceiverPortType>
inline bool Subscriber_t<ReceiverPortType>::getChunk(const mepoo::ChunkHeader** chunkHeader) noexcept
{
    return (m_receiver.getChunk(*chunkHeader));
}

template <typename ReceiverPortType>
inline bool Subscriber_t<ReceiverPortType>::getChunk(const void** payload) noexcept
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

template <typename ReceiverPortType>
inline void Subscriber_t<ReceiverPortType>::deleteNewChunks() noexcept
{
    m_receiver.clearDeliveryFiFo();
}

template <typename ReceiverPortType>
inline bool Subscriber_t<ReceiverPortType>::releaseChunk(const mepoo::ChunkHeader* const chunkHeader) noexcept
{
    return m_receiver.releaseChunk(chunkHeader);
}

template <typename ReceiverPortType>
inline bool Subscriber_t<ReceiverPortType>::releaseChunk(const void* const payload) noexcept
{
    auto chunkHeader = iox::mepoo::convertPayloadPointerToChunkHeader(payload);
    return m_receiver.releaseChunk(chunkHeader);
}

template <typename ReceiverPortType>
inline bool Subscriber_t<ReceiverPortType>::hasNewChunks() const noexcept
{
    return m_receiver.newData();
}

template <typename ReceiverPortType>
inline posix::Semaphore* Subscriber_t<ReceiverPortType>::getSemaphore() const noexcept
{
    // Temporary solution as long as there is no other mechanism to request a semophore
    auto semaphore = m_receiver.GetShmSemaphore();
    assert(semaphore != nullptr && "GetSemaphore: semaphore is not set");

    return semaphore;
}

template <typename ReceiverPortType>
inline void Subscriber_t<ReceiverPortType>::setChunkReceiveSemaphore(posix::Semaphore* semaphore) noexcept
{
    m_receiver.SetCallbackReferences(semaphore);
}

template <typename ReceiverPortType>
inline bool Subscriber_t<ReceiverPortType>::isChunkReceiveSemaphoreSet() noexcept
{
    return m_receiver.AreCallbackReferencesSet();
}

template <typename ReceiverPortType>
inline void Subscriber_t<ReceiverPortType>::unsetChunkReceiveSemaphore() noexcept
{
    m_receiver.UnsetCallbackReferences();
}

template <typename ReceiverPortType>
inline void Subscriber_t<ReceiverPortType>::eventCallbackMain() noexcept
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

template <typename ReceiverPortType>
inline capro::ServiceDescription Subscriber_t<ReceiverPortType>::getServiceDescription() const noexcept
{
    return m_serviceDescription;
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_SUBSCRIBER_INL
