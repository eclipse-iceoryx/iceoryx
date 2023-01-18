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

#ifndef IOX_HOOFS_CXX_ATTRIBUTES_HPP
#define IOX_HOOFS_CXX_ATTRIBUTES_HPP

#include "iceoryx_platform/attributes.hpp"

namespace iox
{
namespace cxx
{
namespace internal
{
/// We use this as an alternative to "static_cast<void>(someVar)" to signal the
/// compiler an unused variable. "static_cast" produces an useless-cast warning
/// on gcc and this approach solves it cleanly.
template <typename T>
inline void IOX_DISCARD_RESULT_IMPL(T&&) noexcept
{
}
} // namespace internal
} // namespace cxx
} // namespace iox

// NOLINTJUSTIFICATION cannot be implemented with a function, required as inline code
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
/// @brief if a function has a return value which you do not want to use then you can wrap the function with that macro.
/// Purpose is to suppress the unused compiler warning by adding an attribute to the return value
/// @param[in] expr name of the function where the return value is not used.
/// @code
///     uint32_t foo();
///     IOX_DISCARD_RESULT(foo()); // suppress compiler warning for unused return value
/// @endcode
#define IOX_DISCARD_RESULT(expr) ::iox::cxx::internal::IOX_DISCARD_RESULT_IMPL(expr)

// NOLINTEND(cppcoreguidelines-macro-usage)


#endif
