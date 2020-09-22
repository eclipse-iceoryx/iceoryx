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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_SAMPLE_HPP
#define IOX_EXPERIMENTAL_POSH_POPO_SAMPLE_HPP

#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

namespace iox
{
namespace popo
{
template <typename T>
class PublisherInterface;

///
/// @brief The Sample class is a mutable abstraction over types which are written to loaned shared memory.
/// These samples are publishable to the iceoryx system.
///
template <typename T>
class Sample
{
  public:
    Sample(cxx::unique_ptr<T>&& samplePtr, PublisherInterface<T>& publisher);
    Sample(const Sample<T>&) = delete;
    Sample<T>& operator=(const Sample<T>&) = delete;
    Sample<T>& operator=(Sample<T>&& rhs);
    Sample(Sample<T>&& rhs);
    ~Sample();

    Sample(std::nullptr_t) noexcept;

    ///
    /// @brief operator -> Transparent access to the underlying pointer.
    /// @return
    ///
    T* operator->() noexcept;

    ///
    /// @brief allocation Access to the memory allocated to the sample.
    /// @return
    ///
    T* get() noexcept;

    ///
    /// @brief header Retrieve the header of the underlying memory chunk used by the sample.
    /// @return The ChunkHeader of the underlying memory chunk.
    ///
    mepoo::ChunkHeader* getHeader() noexcept;

    ///
    /// @brief publish Publish the sample via the publisher from which it was loaned.
    ///
    void publish() noexcept;

  protected:
    cxx::unique_ptr<T> m_samplePtr{nullptr};
    std::reference_wrapper<PublisherInterface<T>> m_publisherRef;
};

///
/// @brief The Sample<const T> class is a non-mutable abstraction over types which are written to loaned shared memory.
/// These samples are received from the iceoryx system via subscribers.
///
template <typename T>
class Sample<const T>
{
  public:
    /// Creates an empty sample.
    Sample(cxx::unique_ptr<T>&& samplePtr) noexcept;
    Sample(const Sample&) = delete;
    Sample& operator=(const Sample&) = delete;
    Sample(Sample<const T>&& rhs);
    Sample& operator=(Sample<const T>&& rhs);
    ~Sample();

    Sample(std::nullptr_t) noexcept;

    const T* operator->() noexcept;
    const T* get() noexcept;
    const mepoo::ChunkHeader* getHeader() noexcept;

  private:
    cxx::unique_ptr<T> m_samplePtr{nullptr};
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/experimental/internal/popo/sample.inl"

#endif // IOX_EXPERIMENTAL_POSH_POPO_SAMPLE_HPP
