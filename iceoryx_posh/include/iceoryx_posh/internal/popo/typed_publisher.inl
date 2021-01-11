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

#ifndef IOX_POSH_POPO_TYPED_PUBLISHER_INL
#define IOX_POSH_POPO_TYPED_PUBLISHER_INL

#include <cstdint>

namespace iox
{
namespace popo
{
template <typename T, typename base_publisher_t>
inline TypedPublisher<T, base_publisher_t>::TypedPublisher(const capro::ServiceDescription& service,
                                                           const PublisherOptions& publisherOptions)
    : base_publisher_t(service, publisherOptions)
{
}

template <typename T, typename base_publisher_t>
inline cxx::expected<Sample<T>, AllocationError> TypedPublisher<T, base_publisher_t>::loan() noexcept
{
    // Call default constructor here to ensure the type is immediately ready to use by the caller.
    // There is a risk that the type will be re-constructed by the user (e.g. by using a placement new in
    // publishResultOf(), however the overhead is considered to be insignificant and worth the additional safety.
    return std::move(base_publisher_t::loan(sizeof(T)).and_then([](auto& sample) { new (sample.get()) T(); }));
}

template <typename T, typename base_publisher_t>
template <typename Callable, typename... ArgTypes>
inline cxx::expected<AllocationError> TypedPublisher<T, base_publisher_t>::publishResultOf(Callable c,
                                                                                           ArgTypes... args) noexcept
{
    static_assert(
        cxx::is_invocable<Callable, T*, ArgTypes...>::value,
        "TypedPublisher<T>::publishResultOf expects a valid callable with a specific signature as the first argument");
    static_assert(cxx::has_signature<Callable, void(T*, ArgTypes...)>::value,
                  "callable provided to TypedPublisher<T>::publishResultOf must have signature void(T*, ArgsTypes...)");

    return loan().and_then([&](auto& sample) {
        c(sample.get(), std::forward<ArgTypes>(args)...);
        sample.publish();
    });
}

template <typename T, typename base_publisher_t>
inline cxx::expected<AllocationError> TypedPublisher<T, base_publisher_t>::publishCopyOf(const T& val) noexcept
{
    return loan().and_then([&](auto& sample) {
        *sample.get() = val; // Copy assignment of value into sample's memory allocation.
        sample.publish();
    });
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_TYPED_PUBLISHER_INL
