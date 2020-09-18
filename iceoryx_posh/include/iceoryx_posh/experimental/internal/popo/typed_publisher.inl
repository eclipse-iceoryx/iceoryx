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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_TYPED_PUBLISHER_INL
#define IOX_EXPERIMENTAL_POSH_POPO_TYPED_PUBLISHER_INL

#include <cstdint>

namespace iox
{
namespace popo
{
template <typename T, typename base_publisher_t>
TypedPublisher<T, base_publisher_t>::TypedPublisher(const capro::ServiceDescription& service)
    : base_publisher_t(service)
{
}

template <typename T, typename base_publisher_t>
inline uid_t TypedPublisher<T, base_publisher_t>::uid() const noexcept
{
    return base_publisher_t::uid();
}

template <typename T, typename base_publisher_t>
inline cxx::expected<PublishableSample<T>, AllocationError> TypedPublisher<T, base_publisher_t>::loan() noexcept
{
    return base_publisher_t::loan(sizeof(T));
}

template <typename T, typename base_publisher_t>
inline void TypedPublisher<T, base_publisher_t>::publish(PublishableSample<T> sample) noexcept
{
    return base_publisher_t::publish(sample);
}

template <typename T, typename base_publisher_t>
template <typename Callable, typename... ArgTypes>
inline cxx::expected<AllocationError> TypedPublisher<T, base_publisher_t>::publishResultOf(Callable c,
                                                                                           ArgTypes... args) noexcept
{
    static_assert(cxx::is_callable<Callable, T*, ArgTypes...>::value,
                  "TypedPublisher<T>::publishResultOf expects a valid callable as the first argument");
    static_assert(cxx::has_signature<Callable, void(T*, ArgTypes...)>::value,
                  "callable provided to TypedPublisher<T>::publishResultOf must have signature void(T*, ArgsTypes...)");

    auto result = loan();
    if (result.has_error())
    {
        return result;
    }
    else
    {
        auto& sample = result.get_value();
        c(sample.get(), std::forward<ArgTypes>(args)...);
        publish(std::move(sample));
        return cxx::success<>();
    }
}

template <typename T, typename base_publisher_t>
inline cxx::expected<AllocationError> TypedPublisher<T, base_publisher_t>::publishCopyOf(const T& val) noexcept
{
    auto result = loan();
    if (result.has_error())
    {
        return result;
    }
    else
    {
        auto sample = std::move(result.get_value());
        *sample.get() = val; // Copy assignment of value into sample's memory allocation.
        publish(std::move(sample));
        return cxx::success<>();
    }
}

template <typename T, typename base_publisher_t>
inline cxx::optional<PublishableSample<T>> TypedPublisher<T, base_publisher_t>::loanPreviousSample() noexcept
{
    return base_publisher_t::loanPreviousSample();
}

template <typename T, typename base_publisher_t>
inline void TypedPublisher<T, base_publisher_t>::offer() noexcept
{
    return base_publisher_t::offer();
}

template <typename T, typename base_publisher_t>
inline void TypedPublisher<T, base_publisher_t>::stopOffer() noexcept
{
    return base_publisher_t::stopOffer();
}

template <typename T, typename base_publisher_t>
inline bool TypedPublisher<T, base_publisher_t>::isOffered() noexcept
{
    return base_publisher_t::isOffered();
}

template <typename T, typename base_publisher_t>
inline bool TypedPublisher<T, base_publisher_t>::hasSubscribers() noexcept
{
    return base_publisher_t::hasSubscribers();
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_TYPED_PUBLISHER_INL
