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
#ifndef IOX_DUST_DESIGN_CREATION_INL
#define IOX_DUST_DESIGN_CREATION_INL

namespace DesignPattern
{
template <typename DerivedClass, typename ErrorType>
inline Creation<DerivedClass, ErrorType>::Creation(Creation&& rhs) noexcept
{
    *this = std::move(rhs);
}

template <typename DerivedClass, typename ErrorType>
Creation<DerivedClass, ErrorType>& Creation<DerivedClass, ErrorType>::operator=(Creation&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_isInitialized = rhs.m_isInitialized;
        m_errorValue = std::move(rhs.m_errorValue);
        rhs.m_isInitialized = false;
    }
    return *this;
}

template <typename DerivedClass, typename ErrorType>
template <typename... Targs>
inline typename Creation<DerivedClass, ErrorType>::result_t
Creation<DerivedClass, ErrorType>::create(Targs&&... args) noexcept
{
    return verify(DerivedClass(std::forward<Targs>(args)...));
}

template <typename DerivedClass, typename ErrorType>
inline bool Creation<DerivedClass, ErrorType>::isInitialized() const noexcept
{
    return m_isInitialized;
}

template <typename DerivedClass, typename ErrorType>
inline typename Creation<DerivedClass, ErrorType>::result_t
Creation<DerivedClass, ErrorType>::verify(DerivedClass&& newObject) noexcept
{
    if (!newObject.m_isInitialized)
    {
        return iox::err(newObject.m_errorValue);
    }

    return iox::ok(std::move(newObject));
}

template <typename DerivedClass, typename ErrorType>
template <typename... Targs>
inline iox::expected<void, ErrorType> Creation<DerivedClass, ErrorType>::placementCreate(void* const memory,
                                                                                         Targs&&... args) noexcept
{
    auto newClass = static_cast<DerivedClass*>(memory);
    new (newClass) DerivedClass(std::forward<Targs>(args)...);

    if (!newClass->m_isInitialized)
    {
        ErrorType errorValue = newClass->m_errorValue;
        newClass->~DerivedClass();
        return iox::err(errorValue);
    }

    return iox::ok();
}


} // namespace DesignPattern

#endif // IOX_DUST_DESIGN_CREATION_INL
