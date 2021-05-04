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

namespace iox
{
namespace concurrent
{
template <typename ElementType, uint64_t Capacity, typename index_t>
ElementType& Buffer<ElementType, Capacity, index_t>::operator[](const index_t index) noexcept
{
    return *toPtr(index);
}

template <typename ElementType, uint64_t Capacity, typename index_t>
const ElementType& Buffer<ElementType, Capacity, index_t>::operator[](const index_t index) const noexcept
{
    return *toPtr(index);
}

template <typename ElementType, uint64_t Capacity, typename index_t>
ElementType* Buffer<ElementType, Capacity, index_t>::ptr(const index_t index) noexcept
{
    return toPtr(index);
}

template <typename ElementType, uint64_t Capacity, typename index_t>
const ElementType* Buffer<ElementType, Capacity, index_t>::ptr(const index_t index) const noexcept
{
    return toPtr(index);
}

template <typename ElementType, uint64_t Capacity, typename index_t>
uint64_t Buffer<ElementType, Capacity, index_t>::capacity() const noexcept
{
    return Capacity;
}

template <typename ElementType, uint64_t Capacity, typename index_t>
ElementType* Buffer<ElementType, Capacity, index_t>::toPtr(index_t index) const noexcept
{
    auto ptr = &(m_buffer[index * sizeof(ElementType)]);
    return reinterpret_cast<ElementType*>(const_cast<byte_t*>(ptr));
}

} // namespace concurrent
} // namespace iox
