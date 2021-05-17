// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_CXX_POOR_MANS_HEAP_INL
#define IOX_HOOFS_CXX_POOR_MANS_HEAP_INL

#include "iceoryx_hoofs/cxx/poor_mans_heap.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>

namespace iox
{
namespace cxx
{
template <typename Interface, size_t TypeSize, size_t TypeAlignment>
PoorMansHeap<Interface, TypeSize, TypeAlignment>::~PoorMansHeap() noexcept
{
    deleteInstance();
}

template <typename Interface, size_t TypeSize, size_t TypeAlignment>
template <typename Type, typename... CTorArgs>
PoorMansHeap<Interface, TypeSize, TypeAlignment>::PoorMansHeap(PoorMansHeapType<Type>, CTorArgs&&... ctorArgs) noexcept
{
    newInstance<Type>(std::forward<CTorArgs>(ctorArgs)...);
}

template <typename Interface, size_t TypeSize, size_t TypeAlignment>
template <typename Type, typename... CTorArgs>
void PoorMansHeap<Interface, TypeSize, TypeAlignment>::newInstance(CTorArgs&&... ctorArgs) noexcept
{
    static_assert(TypeAlignment >= alignof(Type), "Alignment missmatch! No safe instantiation of Type possible!");
    static_assert(TypeSize >= sizeof(Type), "Size missmatch! Not enough space to instantiate Type!");

    deleteInstance();

    m_instance = new (m_heap) Type(std::forward<CTorArgs>(ctorArgs)...);
}

template <typename Interface, size_t TypeSize, size_t TypeAlignment>
void PoorMansHeap<Interface, TypeSize, TypeAlignment>::deleteInstance() noexcept
{
    if (m_instance != nullptr)
    {
        m_instance->~Interface();
        m_instance = nullptr;
    }
}

template <typename Interface, size_t TypeSize, size_t TypeAlignment>
bool PoorMansHeap<Interface, TypeSize, TypeAlignment>::hasInstance() const noexcept
{
    return m_instance != nullptr;
}

template <typename Interface, size_t TypeSize, size_t TypeAlignment>
Interface* PoorMansHeap<Interface, TypeSize, TypeAlignment>::operator->() const noexcept
{
    return m_instance;
}

template <typename Interface, size_t TypeSize, size_t TypeAlignment>
Interface& PoorMansHeap<Interface, TypeSize, TypeAlignment>::operator*() const noexcept
{
    return *m_instance;
}

} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_POOR_MANS_HEAP_INL
