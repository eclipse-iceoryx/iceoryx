// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2021 by AVIN Systems Private Limited All rights reserved.
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

#include "iceoryx_posh/popo/smart_chunk.hpp"

namespace iox
{
namespace popo
{
template <typename T, typename H>
class PublisherInterface;

///
/// @brief The Sample class is a mutable abstraction over types which are written to loaned shared memory.
/// These samples are publishable to the iceoryx system.
///
template <typename T, typename H = cxx::add_const_conditionally_t<mepoo::NoUserHeader, T>>
class Sample : public SmartChunk<PublisherInterface<T, H>, T, H>
{

    using Base_t = SmartChunk<PublisherInterface<T, H>, T, H>;
    /// @brief Helper type to enable the constructor for the producer, i.e. when T has a non const qualifier
    template <typename S, typename TT>
    using ForProducerOnly = typename Base_t::template ForProducerOnly<S, TT>;

    /// @brief Helper type to enable the constructor for the consumer, i.e. when T has a const qualifier
    template <typename S, typename TT>
    using ForConsumerOnly = typename Base_t::template ForConsumerOnly<S, TT>;

    /// @brief Helper type to enable some methods only if a user-header is used
    template <typename R, typename HH>
    using HasUserHeader = typename Base_t::template HasUserHeader<R, HH>;

  public:
    /// @brief Constructor for a Sample used by the Publisher
    /// @tparam S is a dummy template parameter to enable the constructor only for non-const T
    /// @param sampleUniquePtr is a `rvalue` to a `cxx::unique_ptr<T>` with to the data of the encapsulated type T
    /// @param publisher is a reference to the publisher to be able to use the `publish` and `release` methods
    template <typename S = T, typename = ForProducerOnly<S, T>>
    Sample(cxx::unique_ptr<T>&& sampleUniquePtr, PublisherInterface<T, H>& publisher) noexcept;

    /// @brief Constructor for a Sample used by the Subscriber
    /// @tparam S is a dummy template parameter to enable the constructor only for const T
    /// @param sampleUniquePtr is a `rvalue` to a `cxx::unique_ptr<T>` with to the data of the encapsulated type T
    template <typename S = T, typename = ForConsumerOnly<S, T>>
    Sample(cxx::unique_ptr<T>&& sampleUniquePtr) noexcept;

    ~Sample() noexcept = default;

    Sample<T, H>& operator=(Sample<T, H>&& rhs) noexcept = default;
    Sample(Sample<T, H>&& rhs) noexcept = default;

    Sample(const Sample<T, H>&) = delete;
    Sample<T, H>& operator=(const Sample<T, H>&) = delete;

    ///
    /// @brief Retrieve the user-header of the underlying memory chunk loaned to the sample.
    /// @return The user-header of the underlying memory chunk.
    ///
    template <typename R = H, typename = HasUserHeader<R, H>>
    R& getUserHeader() noexcept;

    ///
    /// @brief Retrieve the user-header of the underlying memory chunk loaned to the sample.
    /// @return The user-header of the underlying memory chunk.
    ///
    template <typename R = H, typename = HasUserHeader<R, H>>
    const R& getUserHeader() const noexcept;

    ///
    /// @brief Publish the sample via the publisher from which it was loaned and automatically
    /// release ownership to it.
    /// @details Only available for non-const type T.
    ///
    template <typename S = T, typename = ForProducerOnly<S, T>>
    void publish() noexcept;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/sample.inl"

#endif // IOX_POSH_POPO_SAMPLE_HPP
