// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_SAMPLE_HPP
#define IOX_POSH_POPO_SAMPLE_HPP

#include "iceoryx_posh/internal/popo/smart_chunk.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iox/logging.hpp"
#include "iox/type_traits.hpp"
#include "iox/unique_ptr.hpp"

namespace iox
{
namespace popo
{
template <typename T, typename H>
class PublisherInterface;

/// @brief The Sample class is a mutable abstraction over types which are written to loaned shared memory.
/// These samples are publishable to the iceoryx system.
template <typename T, typename H = add_const_conditionally_t<mepoo::NoUserHeader, T>>
class Sample : public SmartChunk<PublisherInterface<T, H>, T, H>
{
    using BaseType = SmartChunk<PublisherInterface<T, H>, T, H>;

  public:
    template <typename T1, typename T2>
    using ForPublisherOnly = typename BaseType::template ForProducerOnly<T1, T2>;

    /// @brief Constructor for a Sample used by the publisher/subscriber
    /// @tparam S is a dummy template parameter to enable the constructor only for non-const T
    /// @param smartChunkUniquePtr is a 'rvalue' to a 'iox::unique_ptr<T>' with to the data of the encapsulated type
    /// T
    /// @param producer (for publisher only) is a reference to the publisher to be able to use publisher specific
    /// methods
    using BaseType::BaseType;

    /// @brief Retrieve the user-header of the underlying memory chunk loaned to the sample.
    /// @return The user-header of the underlying memory chunk.
    using BaseType::getUserHeader;

    /// @brief Publish the sample via the publisher from which it was loaned and automatically
    /// release ownership to it.
    /// @details Only available for non-const type T.
    template <typename S = T, typename = ForPublisherOnly<S, T>>
    void publish() noexcept;

  private:
    template <typename, typename, typename>
    friend class PublisherImpl;

    /// @note used by the publisher to release the chunk ownership from the 'Sample' after publishing the chunk and
    /// therefore preventing the invocation of the custom deleter
    using BaseType::release;

    using BaseType::m_members;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/sample.inl"

#endif // IOX_POSH_POPO_SAMPLE_HPP
