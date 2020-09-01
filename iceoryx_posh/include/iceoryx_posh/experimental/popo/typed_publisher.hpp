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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_TYPED_PUBLISHER_HPP
#define IOX_EXPERIMENTAL_POSH_POPO_TYPED_PUBLISHER_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/experimental/popo/base_publisher.hpp"
#include "iceoryx_utils/cxx/expected.hpp"


namespace iox
{
namespace popo
{

template<typename T>
class TypedPublisher : protected iox::popo::BasePublisher<T>
{
public:
    ///
    /// @brief Publisher Create publisher for specified service [legacy].
    /// @param service Service to publish to.
    ///
    TypedPublisher(const capro::ServiceDescription& service);

    TypedPublisher(const TypedPublisher& other) = delete;
    TypedPublisher& operator=(const TypedPublisher&) = delete;
    TypedPublisher(TypedPublisher&& rhs) = default;
    TypedPublisher& operator=(TypedPublisher&& rhs) = default;
    ~TypedPublisher() = default;

    ///
    /// @brief loan Loan an empty sample from the shared memory pool.
    /// @return Pointer to the successfully loaned sample, otherwise an allocation error.
    ///
    cxx::expected<Sample<T>, AllocationError> loan() noexcept;
    void release(Sample<T>& sample) noexcept;
    cxx::expected<AllocationError> publish(Sample<T>& sample) noexcept;
    cxx::expected<AllocationError> publishResultOf(cxx::function_ref<void(T*)> f) noexcept;
    cxx::expected<AllocationError> publishCopyOf(const T& val) noexcept; /// @todo - move to typed API

    void offer() noexcept;
    void stopOffer() noexcept;
    bool isOffered() noexcept;
    bool hasSubscribers() noexcept;

};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/experimental/internal/popo/typed_publisher.inl"

#endif // IOX_EXPERIMENTAL_POSH_POPO_TYPED_PUBLISHER_HPP
