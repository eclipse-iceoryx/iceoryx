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
#ifndef IOX_POSH_MEPOO_SHARED_POINTER_HPP
#define IOX_POSH_MEPOO_SHARED_POINTER_HPP

#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iox/expected.hpp"

namespace iox
{
namespace mepoo
{
enum class SharedPointerError
{
    SharedChunkIsEmpty
};

/// @brief A typed shared pointer for the untyped 'SharedChunk'.
/// @code
///     // expected
///     auto sharedPointer = SharedPointer<int>::create(mySharedChunk, 123);
///     if ( sharedPointer.has_error() ) {
///         // ...
///     } else {
///         // ...
///     }
/// @endcode
template <typename T>
class SharedPointer
{
  public:
    template <typename... Targs>
    static expected<SharedPointer<T>, SharedPointerError> create(const SharedChunk& chunk, Targs&&... args) noexcept;

    SharedPointer() = default;
    SharedPointer(const SharedPointer&) = default;
    SharedPointer(SharedPointer&&) = default;
    ~SharedPointer() noexcept;

    SharedPointer& operator=(const SharedPointer&) noexcept;
    SharedPointer& operator=(SharedPointer&&) noexcept;

    T* get() noexcept;
    const T* get() const noexcept;

    T* operator->() noexcept;
    const T* operator->() const noexcept;

    T& operator*() noexcept;
    const T& operator*() const noexcept;

    explicit operator bool() const noexcept;

  private:
    template <typename... Targs>
    SharedPointer(const SharedChunk& chunk, Targs&&... args) noexcept;

    SharedPointer(const SharedChunk& chunk) noexcept;

    void deleteManagedObjectIfNecessary() noexcept;

  private:
    SharedChunk m_chunk;
};

} // namespace mepoo
} // namespace iox

#include "iceoryx_posh/internal/mepoo/shared_pointer.inl"

#endif // IOX_POSH_MEPOO_SHARED_POINTER_HPP
