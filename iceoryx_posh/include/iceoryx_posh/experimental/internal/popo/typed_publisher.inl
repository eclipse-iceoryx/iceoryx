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

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/expected.hpp"

#include <iostream>

namespace iox
{
namespace popo
{

template<typename T>
TypedPublisher<T>::TypedPublisher(const capro::ServiceDescription& service)
    : BasePublisher<T>(service)
{}

template<typename T>
inline cxx::expected<Sample<T>, AllocationError>
TypedPublisher<T>::loan() noexcept
{
    return BasePublisher<T>::loan(sizeof(T));
}

template<typename T>
inline void
TypedPublisher<T>::release(Sample<T>& sample) noexcept
{
    return BasePublisher<T>::release(sample);
}

template<typename T>
inline cxx::expected<AllocationError>
TypedPublisher<T>::publish(Sample<T>& sample) noexcept
{
    return BasePublisher<T>::publish(sample);
}

template<typename T>
inline cxx::expected<AllocationError>
TypedPublisher<T>::publishResultOf(cxx::function_ref<void(T*)> f) noexcept
{
    auto result = loan();
    if(!result.has_error())
    {
        auto& sample = result.get_value();
        f(sample.get());
        return publish(sample);
    }
    else
    {
        return result;
    }
}

template<typename T>
inline cxx::expected<AllocationError>
TypedPublisher<T>::publishCopyOf(const T& val) noexcept
{
//    loan()
//        .and_then([&](Sample<T>& sample){
//            sample.emplace(val);
//            publish(std::move(sample));
//        });
}

template<typename T>
inline void
TypedPublisher<T>::offer() noexcept
{
    return BasePublisher<T>::offer();
}

template<typename T>
inline void
TypedPublisher<T>::stopOffer() noexcept
{
    return BasePublisher<T>::stopOffer();
}

template<typename T>
inline bool
TypedPublisher<T>::isOffered() noexcept
{
    return BasePublisher<T>::isOffered();
}

template<typename T>
inline bool
TypedPublisher<T>::hasSubscribers() noexcept
{
    return BasePublisher<T>::hasSubscribers();
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_TYPED_PUBLISHER_INL
