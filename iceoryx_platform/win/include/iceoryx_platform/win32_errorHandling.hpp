// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT
#ifndef IOX_HOOFS_WIN_PLATFORM_WIN32_ERRORHANDLING_HPP
#define IOX_HOOFS_WIN_PLATFORM_WIN32_ERRORHANDLING_HPP

#include <type_traits>
#include <utility>

#include "iceoryx_platform/windows.hpp"

int __PrintLastErrorToConsole(const char* functionName, const char* file, const int line) noexcept;

template <typename ReturnType>
struct Win32CallReturn
{
    template <typename Function, typename... Targs>
    void assignValue(const Function& f, Targs&&... args)
    {
        value = f(std::forward<Targs>(args)...);
    }
    ReturnType value;
    int error;
};

template <>
struct Win32CallReturn<void>
{
    template <typename Function, typename... Targs>
    void assignValue(const Function& f, Targs&&... args)
    {
        f(std::forward<Targs>(args)...);
    }
    int error;
};

template <typename Function, typename... Targs>
auto __Win32Call(const char* functionName, const char* file, const int line, const Function& f, Targs&&... args)
    -> Win32CallReturn<std::invoke_result_t<decltype(f)&, Targs...>>
{
    Win32CallReturn<std::invoke_result_t<decltype(f)&, Targs...>> retVal;
    SetLastError(0);
    retVal.assignValue(f, std::forward<Targs>(args)...);
    retVal.error = __PrintLastErrorToConsole(functionName, file, line);
    return retVal;
}

#define Win32Call(function, ...) __Win32Call(#function, __FILE__, __LINE__, function, __VA_ARGS__)

#endif // IOX_HOOFS_WIN_PLATFORM_WIN32_ERRORHANDLING_HPP
