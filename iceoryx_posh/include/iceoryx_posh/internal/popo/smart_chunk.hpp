// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_SMART_CHUNK_HPP
#define IOX_POSH_POPO_SMART_CHUNK_HPP

#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iox/type_traits.hpp"
#include "iox/unique_ptr.hpp"

namespace iox
{
namespace popo
{
namespace internal
{
/// @brief helper struct for smartChunk
template <typename TransmissionInterface, typename T, typename H>
struct SmartChunkPrivateData
{
    SmartChunkPrivateData(iox::unique_ptr<T>&& smartChunkUniquePtr, TransmissionInterface& producer) noexcept;

    SmartChunkPrivateData(SmartChunkPrivateData&& rhs) noexcept = default;
    SmartChunkPrivateData& operator=(SmartChunkPrivateData&& rhs) noexcept = default;

    SmartChunkPrivateData(const SmartChunkPrivateData&) = delete;
    SmartChunkPrivateData& operator=(const SmartChunkPrivateData&) = delete;
    ~SmartChunkPrivateData() = default;

    optional<iox::unique_ptr<T>> smartChunkUniquePtr;
    std::reference_wrapper<TransmissionInterface> producerRef;
};

/// @brief specialization of helper struct for smartChunk for const T
template <typename TransmissionInterface, typename T, typename H>
struct SmartChunkPrivateData<TransmissionInterface, const T, H>
{
    explicit SmartChunkPrivateData(iox::unique_ptr<const T>&& smartChunkUniquePtr) noexcept;

    SmartChunkPrivateData(SmartChunkPrivateData&& rhs) noexcept = default;
    SmartChunkPrivateData& operator=(SmartChunkPrivateData&& rhs) noexcept = default;

    SmartChunkPrivateData(const SmartChunkPrivateData&) = delete;
    SmartChunkPrivateData& operator=(const SmartChunkPrivateData&) = delete;
    ~SmartChunkPrivateData() = default;

    optional<iox::unique_ptr<const T>> smartChunkUniquePtr;
};
} // namespace internal


template <typename TransmissionInterface, typename T, typename H = add_const_conditionally_t<mepoo::NoUserHeader, T>>
class SmartChunk
{
  protected:
    static_assert(std::is_const<T>::value == std::is_const<H>::value,
                  "The type 'T' and the user-header 'H' must be equal in their const qualifier to ensure the same "
                  "access restrictions for the user-header as for the smartChunk data!");

    /// @brief Helper type to enable the constructor for the producer, i.e. when T has no const qualifier
    template <typename S, typename TT>
    using ForProducerOnly = std::enable_if_t<std::is_same<S, TT>::value && !std::is_const<TT>::value, S>;

    /// @brief Helper type to enable the constructor for the consumer, i.e. when T has a const qualifier
    template <typename S, typename TT>
    using ForConsumerOnly = std::enable_if_t<std::is_same<S, TT>::value && std::is_const<TT>::value, S>;

    /// @brief Helper type to enable some methods only if a user-header is used
    template <typename R, typename HH>
    using HasUserHeader =
        std::enable_if_t<std::is_same<R, HH>::value && !std::is_same<R, mepoo::NoUserHeader>::value, R>;

  public:
    /// @brief Constructor for a SmartChunk used by the Producer
    /// @tparam S is a dummy template parameter to enable the constructor only for non-const T
    /// @param smartChunkUniquePtr is a 'rvalue' to a 'iox::unique_ptr<T>' with to the data of the encapsulated type
    /// T
    /// @param producer is a reference to the producer to be able to use producer specific methods
    template <typename S = T, typename = ForProducerOnly<S, T>>
    SmartChunk(iox::unique_ptr<T>&& smartChunkUniquePtr, TransmissionInterface& producer) noexcept;

    /// @brief Constructor for a SmartChunk used by the Consumer
    /// @tparam S is a dummy template parameter to enable the constructor only for const T
    /// @param smartChunkUniquePtr is a 'rvalue' to a 'iox::unique_ptr<T>' with to the data of the encapsulated type
    /// T
    template <typename S = T, typename = ForConsumerOnly<S, T>>
    explicit SmartChunk(iox::unique_ptr<T>&& smartChunkUniquePtr) noexcept;

    ~SmartChunk() noexcept = default;

    SmartChunk& operator=(SmartChunk&& rhs) noexcept = default;
    SmartChunk(SmartChunk&& rhs) noexcept = default;

    SmartChunk(const SmartChunk&) = delete;
    SmartChunk& operator=(const SmartChunk&) = delete;

    ///
    /// @brief Transparent access to the encapsulated type.
    /// @return a pointer to the encapsulated type.
    ///
    T* operator->() noexcept;

    ///
    /// @brief Transparent read-only access to the encapsulated type.
    /// @return a const pointer to the encapsulated type.
    ///
    const T* operator->() const noexcept;

    ///
    /// @brief Provides a reference to the encapsulated type.
    /// @return A T& to the encapsulated type.
    ///
    T& operator*() noexcept;

    ///
    /// @brief Provides a const reference to the encapsulated type.
    /// @return A const T& to the encapsulated type.
    ///
    const T& operator*() const noexcept;

    ///
    /// @brief Indicates whether the smartChunk is valid, i.e. refers to allocated memory.
    /// @return true if the smartChunk is valid, false otherwise.
    ///
    explicit operator bool() const noexcept;

    ///
    /// @brief Mutable access to the encapsulated type loaned to the smartChunk.
    /// @return a pointer to the encapsulated type.
    ///
    T* get() noexcept;

    ///
    /// @brief Read-only access to the encapsulated type loaned to the smartChunk.
    /// @return a const pointer to the encapsulated type.
    ///
    const T* get() const noexcept;

    ///
    /// @brief Retrieve the ChunkHeader of the underlying memory chunk loaned to the smartChunk.
    /// @return The ChunkHeader of the underlying memory chunk.
    ///
    add_const_conditionally_t<mepoo::ChunkHeader, T>* getChunkHeader() noexcept;

    ///
    /// @brief Retrieve the ChunkHeader of the underlying memory chunk loaned to the smartChunk.
    /// @return The const ChunkHeader of the underlying memory chunk.
    ///
    const mepoo::ChunkHeader* getChunkHeader() const noexcept;

  protected:
    /// @brief Retrieve the user-header of the underlying memory chunk loaned to the SmartChunk.
    /// @return The user-header of the underlying memory chunk.
    ///
    template <typename R = H, typename = HasUserHeader<R, H>>
    add_const_conditionally_t<R, T>& getUserHeader() noexcept;

    ///
    /// @brief Retrieve the user-header of the underlying memory chunk loaned to the SmartChunk.
    /// @return The user-header of the underlying memory chunk.
    ///
    template <typename R = H, typename = HasUserHeader<R, H>>
    const R& getUserHeader() const noexcept;

    /// @note used by the producer to release the chunk ownership from the 'SmartChunk' after publishing the chunk and
    /// therefore preventing the invocation of the custom deleter
    T* release() noexcept;


  protected:
    internal::SmartChunkPrivateData<TransmissionInterface, T, H> m_members;
};
} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/smart_chunk.inl"

#endif
