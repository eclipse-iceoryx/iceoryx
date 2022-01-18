// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_CXX_FUNCTIONAL_POLICY_INL
#define IOX_HOOFS_CXX_FUNCTIONAL_POLICY_INL

namespace iox
{
namespace cxx
{
namespace functional_policy
{
template <typename T>
inline T& Expect<T>::expect(const char* const msg) & noexcept
{
    T* upcastedThis = static_cast<T*>(this);

    if (!(*upcastedThis))
    {
        std::cout << msg << std::endl;
        Ensures(false);
    }

    return *upcastedThis;
}

template <typename T>
inline const T& Expect<T>::expect(const char* const msg) const& noexcept
{
    return const_cast<const T&>(const_cast<Expect<T>*>(this)->expect(msg));
}

template <typename T>
inline T&& Expect<T>::expect(const char* const msg) && noexcept
{
    return std::move(this->expect(msg));
}

template <typename T>
inline const T&& Expect<T>::expect(const char* const msg) const&& noexcept
{
    return const_cast<const T&&>(std::move(const_cast<Expect<T>*>(this)->expect(msg)));
}
} // namespace functional_policy
} // namespace cxx
} // namespace iox
#endif
