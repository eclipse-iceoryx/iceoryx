// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_EVENT_CALLBACK_INL
#define IOX_POSH_POPO_EVENT_CALLBACK_INL

namespace iox
{
namespace popo
{
template <typename OriginType, typename UserType = internal::NoType_t>
inline EventCallback<OriginType, UserType> createEventCallback(void (&callback)(OriginType* const))
{
    return EventCallback<OriginType, UserType>{&callback};
}

template <typename OriginType, typename UserType>
inline EventCallback<OriginType, UserType> createEventCallback(void (&callback)(OriginType* const, UserType* const),
                                                               UserType& userValue)
{
    return EventCallback<OriginType, UserType>{&callback, &userValue};
}
} // namespace popo
} // namespace iox

#endif
