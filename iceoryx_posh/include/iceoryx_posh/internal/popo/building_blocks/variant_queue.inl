// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by ekxide IO GmbH. All rights reserved.
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

#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_VARIANT_QUEUE_INL
#define IOX_POSH_POPO_BUILDING_BLOCKS_VARIANT_QUEUE_INL

#include "iceoryx_posh/internal/popo/building_blocks/variant_queue.hpp"

namespace iox
{
namespace popo
{
template <typename ValueType, uint64_t Capacity>
inline VariantQueue<ValueType, Capacity>::VariantQueue(const VariantQueueTypes type) noexcept
    : m_type(type)
{
    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        m_fifo.template emplace<concurrent::SpscFifo<ValueType, Capacity>>();
        break;
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        m_fifo.template emplace<concurrent::SpscSofi<ValueType, Capacity>>();
        break;
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
        [[fallthrough]];
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        m_fifo.template emplace<concurrent::MpmcResizeableLockFreeQueue<ValueType, Capacity>>();
        break;
    }
    }
}

template <typename ValueType, uint64_t Capacity>
optional<ValueType> VariantQueue<ValueType, Capacity>::push(const ValueType& value) noexcept
{
    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::FiFo_SingleProducerSingleConsumer)>();
        auto hadSpace = queue->push(value);

        return (hadSpace) ? nullopt : make_optional<ValueType>(value);
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        ValueType overriddenValue;
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>();
        auto hadSpace = queue->push(value, overriddenValue);

        return (hadSpace) ? nullopt : make_optional<ValueType>(overriddenValue);
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>();
        auto hadSpace = queue->tryPush(value);

        return (hadSpace) ? nullopt : make_optional<ValueType>(value);
    }
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>();
        return queue->push(value);
    }
    }

    return nullopt;
}

template <typename ValueType, uint64_t Capacity>
inline optional<ValueType> VariantQueue<ValueType, Capacity>::pop() noexcept
{
    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::FiFo_SingleProducerSingleConsumer)>();
        return queue->pop();
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        ValueType returnType;
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>();
        auto hasReturnType = queue->pop(returnType);

        return (hasReturnType) ? make_optional<ValueType>(returnType) : nullopt;
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>();
        return queue->pop();
    }
    }

    return nullopt;
}

template <typename ValueType, uint64_t Capacity>
inline bool VariantQueue<ValueType, Capacity>::empty() const noexcept
{
    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::FiFo_SingleProducerSingleConsumer)>();
        return queue->empty();
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>();
        return queue->empty();
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>();
        return queue->empty();
    }
    }

    return true;
}

template <typename ValueType, uint64_t Capacity>
inline uint64_t VariantQueue<ValueType, Capacity>::size() noexcept
{
    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::FiFo_SingleProducerSingleConsumer)>();
        return queue->size();
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>();
        return queue->size();
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>();
        return queue->size();
    }
    }

    return 0U;
}


template <typename ValueType, uint64_t Capacity>
inline bool VariantQueue<ValueType, Capacity>::setCapacity(const uint64_t newCapacity) noexcept
{
    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        /// @todo iox-#1147 must be implemented for FiFo
        IOX_PANIC("'setCapacity' for 'SpscFifo' is not yet implemented");
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>();
        queue->setCapacity(newCapacity);
        return true;
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>();
        // we may discard elements in the queue if the size is reduced and the fifo contains too many elements
        return queue->setCapacity(newCapacity);
    }
    }
    return false;
}

template <typename ValueType, uint64_t Capacity>
inline uint64_t VariantQueue<ValueType, Capacity>::capacity() const noexcept
{
    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::FiFo_SingleProducerSingleConsumer)>();
        return queue->capacity();
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>();
        return queue->capacity();
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        // SAFETY: 'm_type' ist 'const' and does not change after construction
        auto* queue = m_fifo.template unsafe_get_at_index_unchecked<static_cast<uint64_t>(
            VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>();
        return queue->capacity();
    }
    }

    return 0U;
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_VARIANT_QUEUE_INL
