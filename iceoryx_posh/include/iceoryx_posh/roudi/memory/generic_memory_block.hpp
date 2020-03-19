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

#pragma once

#include "iceoryx_posh/roudi/memory/memory_block.hpp"

#include "iceoryx_utils/cxx/optional.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
/// @brief The GenericMemoryBlock is an implementation of a MemoryBlock for a common use case.
template <typename T>
class GenericMemoryBlock final : public MemoryBlock
{
    friend class MemoryProvider;

  public:
    GenericMemoryBlock() noexcept = default;
    ~GenericMemoryBlock() noexcept;

    GenericMemoryBlock(const GenericMemoryBlock&) = delete;
    GenericMemoryBlock(GenericMemoryBlock&&) = delete;
    GenericMemoryBlock& operator=(const GenericMemoryBlock&) = delete;
    GenericMemoryBlock& operator=(GenericMemoryBlock&&) = delete;

    /// @brief Implementation of MemoryBlock::size
    /// @return the size of type T
    uint64_t size() const noexcept override;

    /// @brief Implementation of MemoryBlock::alignment
    /// @return the alignment of type T
    uint64_t alignment() const noexcept override;

    /// @brief Implementation of MemoryBlock::destroy
    void destroy() noexcept override;

    /// @brief A new element is constructed by forwarding the arguments to the constructor of T. If the MemoryBlock has
    /// a value then the destructor of T is called.
    /// @param [in] args are perfectly forwarded to the constructor of T to perform a placement new
    /// @return an optional pointer to the underlying type, cxx::nullopt_t if memory was not yet available
    template <typename... Targs>
    cxx::optional<T*> emplace(Targs&&... args) noexcept;

    /// @brief This function enables the access to the underlying type
    /// @return an optional pointer to the underlying type, cxx::nullopt_t if value is not initialized
    cxx::optional<T*> value() const noexcept;

  private:
    T* m_value{nullptr};
};

} // namespace roudi
} // namespace iox

#include "iceoryx_posh/internal/roudi/memory/generic_memory_block.inl"
