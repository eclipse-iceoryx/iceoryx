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
//    ///
//    /// Placement new of the type T in the memory allocated to the sample.
//    ///
//    template <typename Arg, typename... Args>
//    auto emplace(Arg arg, Args... args)
//        -> typename std::enable_if<!std::is_same<std::decay<Arg>, cxx::function_ref<void(T*)>>::value>::type
//    {
//        if(m_samplePtr != nullptr)
//        {
//            new (allocation()) T(std::forward<Arg>(arg), std::forward<Args>(args)...);
//            m_isEmpty = false;
//        }
//    }
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

    uid_t uid() const noexcept;

    // We need to ensure this type is NOT picked when Untyped is used.
    cxx::expected<Sample<T>, AllocationError> loan(uint64_t size) noexcept;

    // We need a specialization of loan for where T = Untyped.
    // The specialization must return a Sample<void>.
    //
    // This is required since we cannot directly use void as the template type.
    //
    // This is not being selected because substituting Untyped in the above function is not an error ... might need to disable it for that type.

    //    template <typename U = T>
    //    cxx::expected<Sample<void>, AllocationError> loan(uint64_t size, typename std::enable_if<std::is_same<U, Untyped>::value>::type* = nullptr) noexcept;

    void release(Sample<T>& sample) noexcept;
    cxx::expected<AllocationError> publish(Sample<T>& sample) noexcept;
    cxx::expected<SampleRecallError> previousSample() const noexcept;

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

template<typename port_t>
class BasePublisher<Untyped, port_t>
{
public:
    using uid_t = uint64_t;

    BasePublisher(const capro::ServiceDescription& service);
    BasePublisher(const BasePublisher& other) = delete;
    BasePublisher& operator=(const BasePublisher&) = delete;
    BasePublisher(BasePublisher&& rhs) = default;
    BasePublisher& operator=(BasePublisher&& rhs) = default;
    ~BasePublisher() = default;

    uid_t uid() const noexcept;
    cxx::expected<Sample<void>, AllocationError> loan(uint64_t size) noexcept;
//    void release(Sample<void>& sample) noexcept;
//    cxx::expected<AllocationError> publish(Sample<void>& sample) noexcept;
//    cxx::expected<AllocationError> publishResultOf(cxx::function_ref<void(void*)> f) noexcept;
//    cxx::expected<SampleRecallError> previousSample() const noexcept;

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
