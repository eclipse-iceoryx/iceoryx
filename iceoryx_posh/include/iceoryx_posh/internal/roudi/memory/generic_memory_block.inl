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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_ROUDI_MEMORY_GENERIC_MEMORY_BLOCK_INL
#define IOX_POSH_ROUDI_MEMORY_GENERIC_MEMORY_BLOCK_INL

namespace iox
{
namespace roudi
{
template <typename T>
GenericMemoryBlock<T>::~GenericMemoryBlock() noexcept
{
    destroy();
}

template <typename T>
uint64_t GenericMemoryBlock<T>::size() const noexcept
{
    return sizeof(T);
}

template <typename T>
uint64_t GenericMemoryBlock<T>::alignment() const noexcept
{
    return alignof(T);
}

template <typename T>
void GenericMemoryBlock<T>::destroy() noexcept
{
    if (m_value)
    {
        m_value->~T();
        m_value = nullptr;
    }
}

template <typename T>
template <typename... Targs>
optional<T*> GenericMemoryBlock<T>::emplace(Targs&&... args) noexcept
{
    destroy();

    auto rawMemory = this->memory();
    if (rawMemory.has_value())
    {
        m_value = new (*rawMemory) T(std::forward<Targs>(args)...);
    }

    return value();
}

template <typename T>
optional<T*> GenericMemoryBlock<T>::value() const noexcept
{
    return m_value ? make_optional<T*>(m_value) : nullopt_t();
}

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_MEMORY_GENERIC_MEMORY_BLOCK_INL
