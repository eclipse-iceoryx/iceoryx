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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_UNTYPED_PUBLISHER_INL
#define IOX_EXPERIMENTAL_POSH_POPO_UNTYPED_PUBLISHER_INL

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/expected.hpp"

#include "iceoryx_posh/experimental/popo/untyped_publisher.hpp"

#include <iostream>

namespace iox
{
namespace popo
{

UntypedPublisher::UntypedPublisher(const capro::ServiceDescription& service)
    : BasePublisher<Untyped>(service)
{}

inline cxx::expected<Sample<void>, AllocationError>
UntypedPublisher::loan(uint64_t size) noexcept
{
    return BasePublisher<Untyped>::loan(size);
}

//inline void
//UntypedPublisher::release(Sample<void>& sample) noexcept
//{
//    return BasePublisher<Untyped>::release(sample);
//}

//inline cxx::expected<AllocationError>
//UntypedPublisher::publish(Sample<void>& sample) noexcept
//{
//    return BasePublisher<Untyped>::publish(sample);
//}

inline void
UntypedPublisher::offer() noexcept
{
    return BasePublisher<Untyped>::offer();
}

inline void
UntypedPublisher::stopOffer() noexcept
{
    return BasePublisher<Untyped>::stopOffer();
}

inline bool
UntypedPublisher::isOffered() noexcept
{
    return BasePublisher<Untyped>::isOffered();
}

inline bool
UntypedPublisher::hasSubscribers() noexcept
{
    return BasePublisher<Untyped>::hasSubscribers();
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_UNTYPED_PUBLISHER_INL
