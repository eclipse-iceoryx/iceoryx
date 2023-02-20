// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_MEMORY_SCOPED_STATIC_HPP
#define IOX_HOOFS_MEMORY_SCOPED_STATIC_HPP

#include "iox/scope_guard.hpp"

namespace iox
{
/// create a ScopeGuard object to cleanup a static optional object at the end of the scope
/// @tparam [in] T memory container which has emplace(...) and reset
/// @tparam [in] CTorArgs ctor types for the object to construct
/// @param [in] memory is a reference to a memory container, e.g. optional
/// @param [in] ctorArgs ctor arguments for the object to construct
/// @return iox::ScopeGuard
template <typename T, typename... CTorArgs>
ScopeGuard makeScopedStatic(T& memory, CTorArgs&&... ctorArgs) noexcept;
} // namespace iox

#include "iox/detail/scoped_static.inl"


#endif // IOX_HOOFS_MEMORY_SCOPED_STATIC_HPP
