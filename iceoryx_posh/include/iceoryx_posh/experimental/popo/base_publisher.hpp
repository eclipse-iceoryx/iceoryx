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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_BASE_PUBLISHER_HPP
#define IOX_EXPERIMENTAL_POSH_POPO_BASE_PUBLISHER_HPP

#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"

namespace iox {
namespace popo {

struct Untyped {};

template<typename T, typename port_t = iox::popo::SenderPort>
class BasePublisher;

template <typename T>
class Sample
{
public:

    Sample(cxx::unique_ptr<T>&& samplePtr, BasePublisher<T>& publisher)
        : m_samplePtr(std::move(samplePtr)), m_publisher(publisher)
    {};

    Sample(std::nullptr_t) noexcept {};

    Sample(const Sample& other) = delete;
    Sample& operator=(const Sample&) = delete;
    Sample(Sample&& rhs) = default;
    Sample& operator=(Sample&& rhs) = default;

    ~Sample()
    {
        m_samplePtr = nullptr;
    }
    ///
    /// @brief operator = Clear the sample.
    /// @return
    ///
    Sample& operator=(std::nullptr_t) noexcept
    {
      m_samplePtr = nullptr;
      m_isEmpty = true;
      m_isValid = false;
      return *this;
    }
    ///
    /// @brief operator -> Transparent access to the underlying pointer.
    /// @return
    ///
    T* operator->() noexcept
    {
        return allocation();
    }
    ///
    /// @brief allocation Access to the memory allocated to the sample.
    /// @return
    ///
    T* allocation() noexcept
    {
        if(m_isValid)
        {
            return m_samplePtr.get();
        }
        else
        {
            return nullptr;
        }
    }
    ///
    /// Placement new of the type T in the memory allocated to the sample.
    ///
    template <typename Arg, typename... Args>
    auto emplace(Arg arg, Args... args)
        -> typename std::enable_if<!std::is_same<std::decay<Arg>, cxx::function_ref<void(T*)>>::value>::type
    {
        if(m_samplePtr != nullptr)
        {
            new (allocation()) T(std::forward<Arg>(arg), std::forward<Args>(args)...);
            m_isEmpty = false;
        }
    }
    ///
    /// @brief publish Publish the sample.
    ///
    void publish() noexcept
    {
        if(m_isValid && !m_isEmpty)
        {
            m_publisher.publish(*this);
            m_samplePtr.release();  // Release ownership of the sample since it has been published.
            m_isValid = false;      // Mark as invalid to prevent re-use.
        }
        else
        {
            /// @todo Notify caller of attempt to publish invalid chunk.
        }
    }

private:
    bool m_isEmpty = true;
    bool m_isValid = true;

    cxx::unique_ptr<T> m_samplePtr = nullptr;
    BasePublisher<T>& m_publisher;
};

enum class SampleRecallError : uint8_t
{
    NO_PREVIOUS_CHUNK,
    CHUNK_ALREADY_CLAIMED
};

template<typename T, typename port_t>
class BasePublisher
{
public:
    using uid_t = uint64_t;

    BasePublisher(const capro::ServiceDescription& service);

    BasePublisher(const BasePublisher& other) = delete;
    BasePublisher& operator=(const BasePublisher&) = delete;
    BasePublisher(BasePublisher&& rhs) = default;
    BasePublisher& operator=(BasePublisher&& rhs) = default;
    ~BasePublisher() = default;

    ///
    /// @brief uid Get the unique id of the publisher.
    /// @return The unique id.
    ///
    uid_t uid() const noexcept;

    ///
    /// @brief loan Loan an empty sample from the shared memory pool.
    /// @return Pointer to the successfully loaned sample, otherwise an allocation error.
    ///
    cxx::expected<Sample<T>, AllocationError> loan(uint64_t size) noexcept;

    ///
    /// @brief release Releases ownership of an unused allocated chunk.
    /// @details The released chunk will be freed as soon as there are no longer any active references
    /// to it in the system.
    /// @param sample
    ///
    void release(Sample<T>& sample) noexcept;

    ///
    /// @brief publish Publishes the loaned sample to all subscribers.
    /// @param sample
    ///
    cxx::expected<AllocationError> publish(Sample<T>& sample) noexcept;
    ///
    /// @brief publish Publishes the argument produced by the given function.
    /// @details Sample is automatically loaned and released.
    /// @param f Function that produces a value T at the provided location.
    ///
    cxx::expected<AllocationError> publishResultOf(cxx::function_ref<void(T*)> f) noexcept;

    ///
    /// @brief publishCopyOf Copy the given sample into a loaned sample and publish it to all subscribers.
    /// @details This method should not be used for larger data types as it includes a copy. For larger data types, it
    /// is preferred to first laon an empty sample and then directly write the data into it (e.g. with a placement new)
    /// rather than to write it elsewhere then copy it in.
    /// @param val The value to publish
    ///
    cxx::expected<AllocationError> publishCopyOf(const T& val) noexcept;

    ///
    /// @brief previousSample Reclaims ownership of a previously published sample if it has not yet been accessed by subscribers.
    /// @return The previously published sample if one exists and is unclaimed, otherwise an error.
    ///
    cxx::expected<SampleRecallError> previousSample() const noexcept;

    /// @todo Make these const by changing the equivalent port methods.
    void offer() noexcept;
    void stopOffer() noexcept;
    bool isOffered() noexcept;
    bool hasSubscribers() noexcept;

protected:


protected:
    port_t m_port{nullptr};
    bool m_useDynamicPayloadSize = true;

private:


};

}
}

#include "iceoryx_posh/experimental/internal/popo/base_publisher.inl"

#endif // IOX_EXPERIMENTAL_POSH_POPO_BASE_PUBLISHER_HPP
