// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_TYPED_PUBLISHER_HPP
#define IOX_POSH_POPO_TYPED_PUBLISHER_HPP

#include "iceoryx_posh/popo/base_publisher.hpp"
#include "iceoryx_posh/popo/sample.hpp"
#include "iceoryx_utils/cxx/type_traits.hpp"

namespace iox
{
namespace popo
{
template <typename T, typename base_publisher_t = BasePublisher<T>>
class TypedPublisher : public base_publisher_t
{
    static_assert(!std::is_void<T>::value, "Type must not be void. Use the UntypedPublisher for void types.");
    static_assert(std::is_default_constructible<T>::value, "The TypedPublisher requires default-constructable types.");

  public:
    TypedPublisher(const capro::ServiceDescription& service,
                   const PublisherOptions& publisherOptions = PublisherOptions());
    TypedPublisher(const TypedPublisher& other) = delete;
    TypedPublisher& operator=(const TypedPublisher&) = delete;
    TypedPublisher(TypedPublisher&& rhs) = default;
    TypedPublisher& operator=(TypedPublisher&& rhs) = default;
    virtual ~TypedPublisher() = default;

    using base_publisher_t::getServiceDescription;
    using base_publisher_t::getUid;
    using base_publisher_t::hasSubscribers;
    using base_publisher_t::isOffered;
    using base_publisher_t::loanPreviousSample;
    using base_publisher_t::offer;
    using base_publisher_t::publish;
    using base_publisher_t::stopOffer;

    cxx::expected<Sample<T>, AllocationError> loan() noexcept;
    ///
    /// @brief publishCopyOf Copy the provided value into a loaned shared memory chunk and publish it.
    /// @param val Value to copy.
    /// @return Error if unable to allocate memory to loan.
    ///
    cxx::expected<AllocationError> publishCopyOf(const T& val) noexcept;
    ///
    /// @brief publishResultOf Loan a sample from memory, execute the provided callable to write to it, then publish it.
    /// @param c Callable with the signature void(T*, ArgTypes...) that write's it's result to T*.
    /// @param args The arguments of the callable.
    /// @return Error if unable to allocate memory to loan.
    ///
    template <typename Callable, typename... ArgTypes>
    cxx::expected<AllocationError> publishResultOf(Callable c, ArgTypes... args) noexcept;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/typed_publisher.inl"

#endif // IOX_POSH_POPO_TYPED_PUBLISHER_HPP
