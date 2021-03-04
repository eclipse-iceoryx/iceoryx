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

#ifndef IOX_POSH_POPO_SAMPLE_HPP
#define IOX_POSH_POPO_SAMPLE_HPP

#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

namespace iox
{
namespace popo
{
template <typename T>
class PublisherInterface;

namespace internal
{
/// @brief helper struct for sample
template <typename T>
struct SamplePrivateData
{
    SamplePrivateData(cxx::unique_ptr<T>&& sampleUniquePtr, PublisherInterface<T>& publisher) noexcept;

    SamplePrivateData(SamplePrivateData&& rhs) noexcept = default;
    SamplePrivateData& operator=(SamplePrivateData&& rhs) noexcept = default;

    SamplePrivateData(const SamplePrivateData&) = delete;
    SamplePrivateData& operator=(const SamplePrivateData&) = delete;

    cxx::unique_ptr<T> sampleUniquePtr;
    std::reference_wrapper<PublisherInterface<T>> publisherRef;
};

/// @brief specialization of helper struct for sample for const T
template <typename T>
struct SamplePrivateData<const T>
{
    SamplePrivateData(cxx::unique_ptr<const T>&& sampleUniquePtr) noexcept;

    SamplePrivateData(SamplePrivateData&& rhs) noexcept = default;
    SamplePrivateData& operator=(SamplePrivateData&& rhs) noexcept = default;

    SamplePrivateData(const SamplePrivateData&) = delete;
    SamplePrivateData& operator=(const SamplePrivateData&) = delete;

    cxx::unique_ptr<const T> sampleUniquePtr;
};
} // namespace internal

///
/// @brief The Sample class is a mutable abstraction over types which are written to loaned shared memory.
/// These samples are publishable to the iceoryx system.
///
template <typename T>
class Sample
{
    template <typename S, typename TT>
    using ForPublisherOnly = std::enable_if_t<std::is_same<S, TT>::value && !std::is_const<S>::value, S>;

    template <typename S, typename TT>
    using ForSubscriberOnly = std::enable_if_t<std::is_same<S, TT>::value && std::is_const<S>::value, S>;

  public:
    /// @brief constructor for a Sample used by the Publisher
    /// @tparam S is a dummy template parameter to enable the constructor only for non-const T
    /// @param sampleUniquePtr is a `rvalue` to a `cxx::unique_ptr<T>` with to the data of the encapsulated type T
    /// @param publisher is a reference to the publisher to be able to use the `publish` and `release` methods
    template <typename S = T, typename = ForPublisherOnly<S, T>>
    Sample(cxx::unique_ptr<T>&& sampleUniquePtr, PublisherInterface<T>& publisher) noexcept;

    /// @brief constructor for a Sample used by the Subscriber
    /// @tparam S is a dummy template parameter to enable the constructor only for const T
    /// @param sampleUniquePtr is a `rvalue` to a `cxx::unique_ptr<T>` with to the data of the encapsulated type T
    template <typename S = T, typename = ForSubscriberOnly<S, T>>
    Sample(cxx::unique_ptr<T>&& sampleUniquePtr) noexcept;

    Sample(std::nullptr_t) noexcept;
    ~Sample() noexcept = default;

    Sample<T>& operator=(Sample<T>&& rhs) noexcept = default;
    Sample(Sample<T>&& rhs) noexcept = default;

    Sample(const Sample<T>&) = delete;
    Sample<T>& operator=(const Sample<T>&) = delete;

    ///
    /// @brief operator -> Transparent access to the encapsulated type.
    /// @return a pointer to the encapsulated type.
    /// @details Only available for non-const type T.
    ///
    template <typename S = T, typename = ForPublisherOnly<S, T>>
    T* operator->() noexcept;

    ///
    /// @brief operator -> Transparent read-only access to the encapsulated type.
    /// @return a const pointer to the encapsulated type.
    ///
    const T* operator->() const noexcept;

    ///
    /// @brief operator* Provide a reference to the encapsulated type.
    /// @return A T& to the encapsulated type.
    /// @details Only available for non-const type T.
    ///
    template <typename S = T, typename = ForPublisherOnly<S, T>>
    S& operator*() noexcept;

    ///
    /// @brief operator* Provide a const reference to the encapsulated type.
    /// @return A const T& to the encapsulated type.
    ///
    const T& operator*() const noexcept;

    ///
    /// @brief operator bool Indciates whether the sample is valid, i.e. refers to allocated memory.
    /// @return true if the sample is valid, false otherwise.
    ///
    operator bool() const noexcept;

    ///
    /// @brief allocation Access to the encapsulated type loaned to the sample.
    /// @return a pointer to the encapsulated type.
    /// @details Only available for non-const type T.
    ///
    template <typename S = T, typename = ForPublisherOnly<S, T>>
    T* get() noexcept;

    ///
    /// @brief allocation Read-only access to the encapsulated type loaned to the sample.
    /// @return a const pointer to the encapsulated type.
    ///
    const T* get() const noexcept;

    ///
    /// @brief header Retrieve the header of the underlying memory chunk loaned to the sample.
    /// @return The ChunkHeader of the underlying memory chunk.
    /// @details Only available for non-const type T.
    ///
    template <typename S = T, typename = ForPublisherOnly<S, T>>
    mepoo::ChunkHeader* getHeader() noexcept;

    ///
    /// @brief header Retrieve the header of the underlying memory chunk loaned to the sample.
    /// @return The const ChunkHeader of the underlying memory chunk.
    ///
    const mepoo::ChunkHeader* getHeader() const noexcept;

    ///
    /// @brief publish Publish the sample via the publisher from which it was loaned and automatically
    /// release ownership to it.
    /// @details Only available for non-const type T.
    ///
    template <typename S = T, typename = ForPublisherOnly<S, T>>
    void publish() noexcept;

  private:
    template <typename, typename>
    friend class Publisher;

    /// @note used by the publisher to release the chunk ownership from the `Sample` after publishing the chunk and
    /// therefore preventing the invocation of the custom deleter
    T* release() noexcept;

  private:
    internal::SamplePrivateData<T> m_members;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/sample.inl"

#endif // IOX_POSH_POPO_SAMPLE_HPP
