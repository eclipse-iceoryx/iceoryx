// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_UNTYPED_PUBLISHER_HPP
#define IOX_POSH_POPO_UNTYPED_PUBLISHER_HPP

#include "iceoryx_posh/popo/base_publisher.hpp"
#include "iceoryx_posh/popo/sample.hpp"

namespace iox
{
namespace popo
{
template <typename base_publisher_t = BasePublisher<>>
class UntypedPublisherImpl : public base_publisher_t
{
  public:
    UntypedPublisherImpl(const capro::ServiceDescription& service,
                         const PublisherOptions& publisherOptions = PublisherOptions());
    UntypedPublisherImpl(const UntypedPublisherImpl& other) = delete;
    UntypedPublisherImpl& operator=(const UntypedPublisherImpl&) = delete;
    UntypedPublisherImpl(UntypedPublisherImpl&& rhs) = default;
    UntypedPublisherImpl& operator=(UntypedPublisherImpl&& rhs) = default;
    virtual ~UntypedPublisherImpl() = default;

    ///
    /// @brief loan Get a chunk from loaned shared memory.
    /// @param size The expected size of the chunk.
    /// @return A pointer to a chunk of memory with the requested size or
    ///         an AllocationError if no chunk could be loaned.
    /// @note An AllocationError occurs if no chunk is available in the shared memory.
    ///
    cxx::expected<void*, AllocationError> loan(const uint32_t size) noexcept;

    ///
    /// @brief loanPreviousChunk Get the previously loaned chunk if possible.
    /// @return A pointer to the previous chunk if available, nullopt otherwise.
    ///
    cxx::optional<void*> loanPreviousChunk() noexcept;

    ///
    /// @brief publish Publish the provided memory chunk.
    /// @param chunk Pointer to the allocated shared memory chunk.
    /// @return Error if provided pointer is not a valid memory chunk.
    ///
    void publish(const void* chunk) noexcept;

    ///
    /// @brief Releases the ownership of the chunk provided by the payload pointer.
    /// @param chunk pointer to the payload of the chunk to be released
    /// @details The chunk must have been previously provided by loan or loanPreviousChunk
    ///          and not have been already released.
    ///          The chunk must not be accessed afterwards as its memory may have
    ///          been reclaimed.
    ///
    void release(const void* chunk) noexcept;

  protected:
    using base_publisher_t::port;
};

using UntypedPublisher = UntypedPublisherImpl<>;

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/untyped_publisher.inl"

#endif // IOX_POSH_POPO_UNTYPED_PUBLISHER_HPP
