// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_MOCKS_MOCKS_HPP
#define IOX_HOOFS_MOCKS_MOCKS_HPP

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "iceoryx_platform/dlfcn.hpp"

#include <cassert>
#include <string>

namespace mocks
{
template <typename T>
void loadSymbol(T& destination, const std::string& functionName);
template <typename T>
T assignSymbol(const std::string& functionName);
} // namespace mocks


// NOLINTBEGIN(cppcoreguidelines-macro-usage) The intended functionality cannot be achieved with templates
#define STATIC_FUNCTION_LOADER_MANUAL_DEDUCE(type, functionName)                                                       \
    []() {                                                                                                             \
        static auto returnValue = mocks::assignSymbol<type>(#functionName);                                            \
        return returnValue;                                                                                            \
    }()

// NOLINTBEGIN(bugprone-macro-parentheses) The parameter is a function name and the reference to that function cannot be enclosed in parentheses
#define STATIC_FUNCTION_LOADER_AUTO_DEDUCE(functionName)                                                               \
    STATIC_FUNCTION_LOADER_MANUAL_DEDUCE(decltype(&functionName), functionName)
// NOLINTEND(bugprone-macro-parentheses)
// NOLINTEND(cppcoreguidelines-macro-usage)

#include "mocks.inl"

#endif // IOX_HOOFS_MOCKS_MOCKS_HPP
