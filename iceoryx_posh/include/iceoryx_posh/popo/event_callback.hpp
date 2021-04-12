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

#ifndef IOX_POSH_POPO_EVENT_CALLBACK_HPP
#define IOX_POSH_POPO_EVENT_CALLBACK_HPP

namespace iox
{
namespace popo
{
namespace internal
{
struct NoType_t
{
};
} // namespace internal

using GenericCallbackPtr_t = void (*)();
using GenericCallbackRef_t = void (&)();

///@brief the struct describes a callback with a user defined type which can
///         be attached to a WaitSet or a Listener
template <typename OriginType, typename UserType>
struct EventCallback
{
    using Ref_t = void (&)(OriginType* const, UserType* const);
    using Ptr_t = void (*)(OriginType* const, UserType* const);

    Ptr_t m_callback = nullptr;
    UserType* m_userValue = nullptr;
};

///@brief the struct describes a callback which can be attached to a WaitSet or a Listener
template <typename OriginType>
struct EventCallback<OriginType, internal::NoType_t>
{
    using Ref_t = void (&)(OriginType* const);
    using Ptr_t = void (*)(OriginType* const);

    Ptr_t m_callback = nullptr;
    internal::NoType_t* m_userValue = nullptr;
};

/// @brief creates an EventCallback
/// @param[in] callback reference to a callback with the signature void(OriginType*)
/// @return the callback stored inside of an EventCallback
template <typename OriginType, typename UserType = internal::NoType_t>
EventCallback<OriginType, UserType> createEventCallback(void (&callback)(OriginType* const));

/// @brief creates an EventCallback with a user defined value
/// @param[in] callback reference to a callback with the signature void(OriginType*, UserType*)
/// @param[in] userValue reference to a user defined value
/// @return the callback and user value stored inside of an EventCallback
template <typename OriginType, typename UserType>
EventCallback<OriginType, UserType> createEventCallback(void (&callback)(OriginType* const, UserType* const),
                                                        UserType& userValue);
} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/event_callback.inl"

#endif
