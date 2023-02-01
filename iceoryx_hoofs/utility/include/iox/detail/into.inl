// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_UTILITY_INTO_INL
#define IOX_HOOFS_UTILITY_INTO_INL

#include "iox/into.hpp"

#include "iceoryx_hoofs/cxx/type_traits.hpp"

namespace iox
{
template <typename SourceType, typename DestinationType>
inline constexpr DestinationType from(const SourceType value)
{
    return FromImpl<SourceType, DestinationType>::fromImpl(value);
}


template <typename SourceType, typename DestinationType>
inline DestinationType FromImpl<SourceType, DestinationType>::fromImpl(const SourceType&)
{
    static_assert(cxx::always_false_v<SourceType> && cxx::always_false_v<DestinationType>,
                  "Conversion for the specified types is not implemented!\
    Please specialize 'template <typename SourceType, typename DestinationType> constexpr DestinationType FromImpl::fromImpl(const SourceType&) noexcept'!");
}

template <typename DestinationType, typename SourceType>
inline constexpr DestinationType into(const SourceType value)
{
    return from<SourceType, DestinationType>(value);
}
} // namespace iox

#endif // IOX_HOOFS_UTILITY_INTO_INL
