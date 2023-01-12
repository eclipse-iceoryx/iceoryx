// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_MEMORY_GENERIC_MEMORY_BLOCK_HPP
#define IOX_POSH_ROUDI_MEMORY_GENERIC_MEMORY_BLOCK_HPP

#include "iceoryx_posh/roudi/memory/memory_block.hpp"

#include "iox/optional.hpp"

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

    /// @copydoc MemoryBlock::size()
    /// @note The size of the underlying type T
    uint64_t size() const noexcept override;

    /// @copydoc MemoryBlock::alignment
    /// @note The alignment of the underlying type T
    uint64_t alignment() const noexcept override;

    /// @brief A new element is constructed by forwarding the arguments to the constructor of T. If the MemoryBlock has
    /// a value then the destructor of T is called.
    /// @param [in] args are perfectly forwarded to the constructor of T to perform a placement new
    /// @return an optional pointer to the underlying type, nullopt_t if memory was not yet available
    template <typename... Targs>
    optional<T*> emplace(Targs&&... args) noexcept;

    /// @brief This function enables the access to the underlying type
    /// @return an optional pointer to the underlying type, nullopt_t if value is not initialized
    optional<T*> value() const noexcept;

  protected:
    /// @copydoc MemoryBlock::destroy
    /// @note This will destroy the underlying type T
    void destroy() noexcept override;

  private:
    T* m_value{nullptr};
};

} // namespace roudi
} // namespace iox

#include "iceoryx_posh/internal/roudi/memory/generic_memory_block.inl"

#endif // IOX_POSH_ROUDI_MEMORY_GENERIC_MEMORY_BLOCK_HPP
