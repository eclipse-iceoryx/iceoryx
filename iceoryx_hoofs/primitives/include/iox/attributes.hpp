// Copyright (c) 2021-2023 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_PRIMITIVES_ATTRIBUTES_HPP
#define IOX_HOOFS_PRIMITIVES_ATTRIBUTES_HPP

#include "iceoryx_platform/attributes.hpp"

namespace iox
{
namespace internal
{
/// We use this as an alternative to "static_cast<void>(someVar)" to signal the
/// compiler an unused variable. "static_cast" produces an useless-cast warning
/// on gcc and this approach solves it cleanly.
template <typename T>
// AXIVION Next Construct AutosarC++19_03-M0.1.8 : No side effects are the intended behavior of the function
inline void IOX_DISCARD_RESULT_IMPL(T&&) noexcept
{
}
} // namespace internal
} // namespace iox

// AXIVION Next Construct AutosarC++19_03-A16.0.1 : Aliasing of fully qualified templated function.
//                                                  Improves readability. No risks apparent.
// NOLINTJUSTIFICATION cannot be implemented with a function, required as inline code
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
/// @brief if a function has a return value which you do not want to use then you can wrap the function with that macro.
/// Purpose is to suppress the unused compiler warning by adding an attribute to the return value
/// @param[in] expr name of the function where the return value is not used.
/// @code
///     uint32_t foo();
///     IOX_DISCARD_RESULT(foo()); // suppress compiler warning for unused return value
/// @endcode
#define IOX_DISCARD_RESULT(expr) ::iox::internal::IOX_DISCARD_RESULT_IMPL(expr)

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif
