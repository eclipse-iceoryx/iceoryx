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

#pragma once

#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_utils/design_pattern/creation.hpp"

namespace iox
{
namespace mepoo
{
enum class SharedPointerError
{
    SharedChunkIsEmpty
};

/// @brief DesignPattern::Creation offers us a create method which forwards the arguments to
/// the constructor. Use this class like in the code example below
/// @code
///     // cxx::expected
///     auto sharedPointer = SharedPointer<int>::Create(mySharedChunk, 123);
///     if ( sharedPointer.has_error() ) {
///         // ...
///     } else {
///         // ...
///     }
/// @endcode
template <typename T>
class SharedPointer : public DesignPattern::Creation<SharedPointer<T>, SharedPointerError>
{
  public:
    SharedPointer() = default;
    SharedPointer(const SharedPointer&) = default;
    SharedPointer(SharedPointer&&) = default;
    ~SharedPointer();

    SharedPointer& operator=(const SharedPointer&);
    SharedPointer& operator=(SharedPointer&&);

    T* get();
    const T* get() const;

    T* operator->();
    const T* operator->() const;

    T& operator*();
    const T& operator*() const;

    friend class DesignPattern::Creation<SharedPointer<T>, SharedPointerError>;

  private:
    template <typename... Targs>
    SharedPointer(const SharedChunk& chunk, Targs&&... args);

    void deleteManagedObjectIfNecessary();
    SharedChunk m_chunk;
};

} // namespace mepoo
} // namespace iox

#include "iceoryx_posh/internal/mepoo/shared_pointer.inl"

