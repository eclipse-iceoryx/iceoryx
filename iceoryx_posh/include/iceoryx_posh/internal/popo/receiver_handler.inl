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

namespace iox
{
namespace popo
{
template <uint32_t MaxReceivers, typename LockingPolicy>
inline ReceiverHandler<MaxReceivers, LockingPolicy>::AppContext::AppContext(ReceiverHandler_t& f_receiverHandler)
    : m_receiverHandler(f_receiverHandler)
{
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline bool ReceiverHandler<MaxReceivers, LockingPolicy>::AppContext::hasLastChunk()
{
    return m_receiverHandler.hasLastChunk();
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline void ReceiverHandler<MaxReceivers, LockingPolicy>::AppContext::deliverChunk(const mepoo::SharedChunk f_chunk)
{
    m_receiverHandler.deliverChunk(f_chunk);
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline void ReceiverHandler<MaxReceivers, LockingPolicy>::AppContext::updateLastChunk(const mepoo::SharedChunk f_chunk)
{
    m_receiverHandler.updateLastChunk(f_chunk);
}


template <uint32_t MaxReceivers, typename LockingPolicy>
inline bool ReceiverHandler<MaxReceivers, LockingPolicy>::AppContext::hasReceivers()
{
    return m_receiverHandler.hasReceivers();
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline void ReceiverHandler<MaxReceivers, LockingPolicy>::AppContext::enableDoDeliverOnSubscription()
{
    m_receiverHandler.enableDoDeliverOnSubscription();
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline typename ReceiverHandler<MaxReceivers, LockingPolicy>::ReceiverVector_t&
ReceiverHandler<MaxReceivers, LockingPolicy>::AppContext::getReceiverList() noexcept
{
    return m_receiverHandler.getReceiverList();
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline ReceiverHandler<MaxReceivers, LockingPolicy>::RouDiContext::RouDiContext(ReceiverHandler_t& f_receiverHandler)
    : m_receiverHandler(f_receiverHandler)
{
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline bool ReceiverHandler<MaxReceivers, LockingPolicy>::RouDiContext::addNewReceiver(
    ReceiverPortType::MemberType_t* const f_receiver)
{
    return m_receiverHandler.addNewReceiver(f_receiver);
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline void ReceiverHandler<MaxReceivers, LockingPolicy>::RouDiContext::removeReceiver(
    ReceiverPortType::MemberType_t* const f_receiver)
{
    m_receiverHandler.removeReceiver(f_receiver);
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline void ReceiverHandler<MaxReceivers, LockingPolicy>::RouDiContext::removeAll()
{
    m_receiverHandler.removeAll();
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline void ReceiverHandler<MaxReceivers, LockingPolicy>::deliverChunk(const mepoo::SharedChunk f_chunk)
{
    lockGuard_t lock(*this);
    /// @todo use this once the cxx vector has reverse interators
    // std::for_each(m_receiverVector.rbegin(), m_receiverVector.rend(), [&](ReceiverPortData* receiverData) {
    //     ReceiverPort(receiverData).deliver(f_chunk);
    // });

    for (int64_t i = m_receiverVector.size() - 1; i >= 0; --i)
    {
        ReceiverPortType(m_receiverVector[i]).deliver(f_chunk);
    }

    if (m_doDeliverOnSubscription.load(std::memory_order_relaxed))
    {
        m_lastChunk = f_chunk;
    }
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline void ReceiverHandler<MaxReceivers, LockingPolicy>::updateLastChunk(const mepoo::SharedChunk f_chunk)
{
    lockGuard_t lock(*this);

    if (m_doDeliverOnSubscription.load(std::memory_order_relaxed))
    {
        m_lastChunk = f_chunk;
    }
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline bool ReceiverHandler<MaxReceivers, LockingPolicy>::hasLastChunk()
{
    lockGuard_t lock(*this);
    return m_lastChunk != nullptr;
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline bool ReceiverHandler<MaxReceivers, LockingPolicy>::hasReceivers()
{
    lockGuard_t lock(*this);
    return !m_receiverVector.empty();
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline bool
ReceiverHandler<MaxReceivers, LockingPolicy>::addNewReceiver(ReceiverPortType::MemberType_t* const f_receiver)
{
    lockGuard_t lock(*this);
    auto l_alreadyKnownReceiver =
        std::find_if(m_receiverVector.begin(), m_receiverVector.end(), [&](ReceiverPortType::MemberType_t* receiver) {
            return receiver == f_receiver;
        });

    // check if the receiver port is not yet subscribed
    if (l_alreadyKnownReceiver == m_receiverVector.end())
    {
        if (m_receiverVector.size() < m_receiverVector.capacity())
        {
            m_receiverVector.push_back(f_receiver);

            if (m_doDeliverOnSubscription.load(std::memory_order_relaxed))
            {
                if (m_lastChunk != nullptr)
                {
                    ReceiverPortType(f_receiver).deliver(m_lastChunk);
                }
                else
                {
                    errorHandler(Error::kPOSH__SENDERPORT_FIELD_SUBSCRIBE_WITHOUT_DATA);
                    return false;
                }
            }
        }
        else
        {
            errorHandler(Error::kPOSH__SENDERPORT_SUBSCRIBER_LIST_OVERFLOW);
            return false;
        }
    }

    return true;
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline void
ReceiverHandler<MaxReceivers, LockingPolicy>::removeReceiver(ReceiverPortType::MemberType_t* const f_receiver)
{
    lockGuard_t lock(*this);
    auto l_iter = std::find(m_receiverVector.begin(), m_receiverVector.end(), f_receiver);
    if (l_iter != m_receiverVector.end())
    {
        m_receiverVector.erase(l_iter);
    }
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline void ReceiverHandler<MaxReceivers, LockingPolicy>::removeAll()
{
    lockGuard_t lock(*this);
    m_receiverVector.clear();
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline void ReceiverHandler<MaxReceivers, LockingPolicy>::enableDoDeliverOnSubscription()
{
    m_doDeliverOnSubscription.store(true, std::memory_order_relaxed);
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline bool ReceiverHandler<MaxReceivers, LockingPolicy>::doesDeliverOnSubscribe() const
{
    return m_doDeliverOnSubscription.load(std::memory_order_relaxed);
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline typename ReceiverHandler<MaxReceivers, LockingPolicy>::ReceiverVector_t&
ReceiverHandler<MaxReceivers, LockingPolicy>::getReceiverList() noexcept
{
    return m_receiverVector;
}

template <uint32_t MaxReceivers, typename LockingPolicy>
inline uint32_t ReceiverHandler<MaxReceivers, LockingPolicy>::getMaxDeliveryFiFoCapacity()
{
    lockGuard_t lock(*this);

    uint64_t maxDeliveryFiFoCapacity = 0;

    for (auto receiver : m_receiverVector)
    {
        ReceiverPort port(receiver);
        auto deliveryFiFoCapacity = port.getDeliveryFiFoCapacity();
        if (deliveryFiFoCapacity > maxDeliveryFiFoCapacity)
        {
            maxDeliveryFiFoCapacity = deliveryFiFoCapacity;
        }
    }

    return static_cast<uint32_t>(maxDeliveryFiFoCapacity);
}

} // namespace popo
} // namespace iox
