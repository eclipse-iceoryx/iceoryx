// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_UTILS_CXX_VARIANT_QUEUE_INL
#define IOX_UTILS_CXX_VARIANT_QUEUE_INL

#include "iceoryx_utils/error_handling/error_handling.hpp"

namespace iox
{
namespace cxx
{
template <typename ValueType, uint64_t Capacity>
inline VariantQueue<ValueType, Capacity>::VariantQueue(const VariantQueueTypes type) noexcept
    : m_type(type)
{
    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        m_fifo.template emplace<concurrent::FiFo<ValueType, Capacity>>();
        break;
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        m_fifo.template emplace<concurrent::SoFi<ValueType, Capacity>>();
        break;
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        m_fifo.template emplace<concurrent::LockFreeQueue<ValueType, Capacity>>();
        break;
    }
    default:
    {
        errorHandler(Error::kVARIANT_QUEUE__UNSUPPORTED_QUEUE_TYPE);
    }
    }
}

template <typename ValueType, uint64_t Capacity>
inline expected<optional<ValueType>, VariantQueueError>
VariantQueue<ValueType, Capacity>::push(const ValueType& value) noexcept
{
    expected<optional<ValueType>, VariantQueueError> ret = error<VariantQueueError>(VariantQueueError::InternalError);

    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        auto hasSuccess =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::FiFo_SingleProducerSingleConsumer)>()
                ->push(value);

        if (hasSuccess)
        {
            ret = success<optional<ValueType>>(nullopt_t());
        }
        else
        {
            ret = error<VariantQueueError>(VariantQueueError::QueueIsFull);
        }
        break;
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        ValueType overriddenValue;
        auto notskipped =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>()
                ->push(value, overriddenValue);
        if (notskipped)
        {
            ret = success<optional<ValueType>>(nullopt_t());
        }
        else
        {
            ret = success<optional<ValueType>>(optional<ValueType>(std::move(overriddenValue)));
        }
        break;
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    {
        auto hasSuccess =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>()
                ->tryPush(value);
        if (hasSuccess)
        {
            ret = success<optional<ValueType>>(nullopt_t());
        }
        else
        {
            ret = error<VariantQueueError>(VariantQueueError::QueueIsFull);
        }
        break;
    }
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        auto overriddenValue =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>()
                ->push(value);
        if (overriddenValue)
        {
            ret = success<optional<ValueType>>(optional<ValueType>(std::move(overriddenValue.value())));
        }
        else
        {
            ret = success<optional<ValueType>>(nullopt_t());
        }
        break;
    }
    default:
    {
        errorHandler(Error::kVARIANT_QUEUE__UNSUPPORTED_QUEUE_TYPE);
    }
    }

    return ret;
}

template <typename ValueType, uint64_t Capacity>
inline optional<ValueType> VariantQueue<ValueType, Capacity>::pop() noexcept
{
    optional<ValueType> ret = nullopt_t();

    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        ret =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::FiFo_SingleProducerSingleConsumer)>()
                ->pop();
        break;
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        ValueType returnType;
        auto hasReturnType =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>()
                ->pop(returnType);
        if (hasReturnType)
        {
            ret = returnType;
        }
        break;
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        auto maybeReturnType =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>()
                ->pop();
        if (maybeReturnType)
        {
            ret = maybeReturnType.value();
        }
        break;
    }
    default:
    {
        errorHandler(Error::kVARIANT_QUEUE__UNSUPPORTED_QUEUE_TYPE);
    }
    }

    return ret;
}

template <typename ValueType, uint64_t Capacity>
inline bool VariantQueue<ValueType, Capacity>::empty() const noexcept
{
    bool ret = true;

    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        ret =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::FiFo_SingleProducerSingleConsumer)>()
                ->empty();
        break;
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        ret =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>()
                ->empty();
        break;
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        ret =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>()
                ->empty();
        break;
    }
    default:
    {
        errorHandler(Error::kVARIANT_QUEUE__UNSUPPORTED_QUEUE_TYPE);
    }
    }

    return ret;
}

template <typename ValueType, uint64_t Capacity>
inline uint64_t VariantQueue<ValueType, Capacity>::size() noexcept
{
    uint64_t ret = 0;

    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        /// @todo must be implemented for FiFo
        assert(false);
        break;
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        ret =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>()
                ->size();
        break;
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        ret =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>()
                ->size();
        break;
    }
    default:
    {
        errorHandler(Error::kVARIANT_QUEUE__UNSUPPORTED_QUEUE_TYPE);
    }
    }

    return ret;
}


template <typename ValueType, uint64_t Capacity>
inline void VariantQueue<ValueType, Capacity>::setCapacity(const uint64_t newCapacity) noexcept
{
    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        /// @todo must be implemented for FiFo
        assert(false);
        break;
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>()
            ->setCapacity(newCapacity);
        break;
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        /// @todo must be implemented for LockFreeQueue
        assert(false);
        break;
    }
    default:
    {
        errorHandler(Error::kVARIANT_QUEUE__UNSUPPORTED_QUEUE_TYPE);
    }
    }
}

template <typename ValueType, uint64_t Capacity>
inline uint64_t VariantQueue<ValueType, Capacity>::capacity() const noexcept
{
    uint64_t ret = 0;

    switch (m_type)
    {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer:
    {
        /// @todo must be implemented for FiFo
        assert(false);
        break;
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer:
    {
        ret =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>()
                ->capacity();
        break;
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer:
    {
        ret =
            m_fifo.template get_at_index<static_cast<uint64_t>(VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>()
                ->capacity();
        break;
    }
    default:
    {
        errorHandler(Error::kVARIANT_QUEUE__UNSUPPORTED_QUEUE_TYPE);
    }
    }

    return ret;
}

template <typename ValueType, uint64_t Capacity>
inline typename VariantQueue<ValueType, Capacity>::fifo_t&
VariantQueue<ValueType, Capacity>::getUnderlyingFiFo() noexcept
{
    return m_fifo;
}

} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_CXX_VARIANT_QUEUE_INL
