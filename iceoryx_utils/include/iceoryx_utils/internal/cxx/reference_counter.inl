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

#include "iceoryx_utils/internal/cxx/reference_counter.hpp"

namespace iox
{
namespace cxx
{
template <typename T>
inline ReferenceCounter<T>::ReferenceCounter(T* const referenceCounter) noexcept
    : m_referenceCounter(referenceCounter)
{
    incrementReferenceCounter();
}

template <typename T>
inline ReferenceCounter<T>::ReferenceCounter(const ReferenceCounter& rhs) noexcept
    : m_referenceCounter(rhs.m_referenceCounter)
{
    incrementReferenceCounter();
}

template <typename T>
inline ReferenceCounter<T>::ReferenceCounter(ReferenceCounter&& rhs) noexcept
    : m_referenceCounter(rhs.m_referenceCounter)
{
    rhs.m_referenceCounter = nullptr;
}

template <typename T>
inline ReferenceCounter<T>::~ReferenceCounter() noexcept
{
    decrementReferenceCounter();
}

template <typename T>
inline ReferenceCounter<T>& ReferenceCounter<T>::operator=(const ReferenceCounter& rhs) noexcept
{
    if (this != &rhs)
    {
        decrementReferenceCounter();
        m_referenceCounter = rhs.m_referenceCounter;
        incrementReferenceCounter();
    }
    return *this;
}

template <typename T>
inline ReferenceCounter<T>& ReferenceCounter<T>::operator=(ReferenceCounter&& rhs) noexcept
{
    if (this != &rhs)
    {
        decrementReferenceCounter();
        m_referenceCounter = rhs.m_referenceCounter;
        rhs.m_referenceCounter = nullptr;
    }
    return *this;
}

template <typename T>
inline T ReferenceCounter<T>::getValue() const noexcept
{
    return *m_referenceCounter;
}

template <typename T>
inline void ReferenceCounter<T>::incrementReferenceCounter() noexcept
{
    if (m_referenceCounter != nullptr)
    {
        (*m_referenceCounter)++;
    }
}

template <typename T>
inline void ReferenceCounter<T>::decrementReferenceCounter() noexcept
{
    if (m_referenceCounter != nullptr)
    {
        (*m_referenceCounter)--;
    }
}
} // namespace cxx
} // namespace iox
