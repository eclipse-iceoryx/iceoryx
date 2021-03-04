// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

namespace iox
{
namespace cxx
{
template <class ReturnType, class... ArgTypes>
inline function_ref<ReturnType(ArgTypes...)>::function_ref() noexcept
    : m_pointerToCallable(nullptr)
    , m_functionPointer(nullptr)
{
}

template <class ReturnType, class... ArgTypes>
template <typename CallableType, typename, typename>
inline function_ref<ReturnType(ArgTypes...)>::function_ref(CallableType&& callable) noexcept
    : m_pointerToCallable(reinterpret_cast<void*>(std::addressof(callable)))
    , m_functionPointer([](void* target, ArgTypes... args) -> ReturnType {
        return (*reinterpret_cast<typename std::add_pointer<CallableType>::type>(target))(
            std::forward<ArgTypes>(args)...);
    })
{
}

template <class ReturnType, class... ArgTypes>
inline function_ref<ReturnType(ArgTypes...)>::function_ref(function_ref&& rhs) noexcept
{
    *this = std::move(rhs);
}

template <class ReturnType, class... ArgTypes>
inline function_ref<ReturnType(ArgTypes...)>&
function_ref<ReturnType(ArgTypes...)>::operator=(function_ref<ReturnType(ArgTypes...)>&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_pointerToCallable = rhs.m_pointerToCallable;
        m_functionPointer = rhs.m_functionPointer;
        // Make sure no undefined behavior can happen by marking the rhs as invalid
        rhs.m_pointerToCallable = nullptr;
        rhs.m_functionPointer = nullptr;
    }
    return *this;
}

template <class ReturnType, class... ArgTypes>
inline ReturnType function_ref<ReturnType(ArgTypes...)>::operator()(ArgTypes... args) const noexcept
{
    if (!m_pointerToCallable)
    {
        // Callable was called without user having assigned one beforehand
        std::cerr << "Empty function_ref invoked" << std::endl;
        std::terminate();
    }
    return m_functionPointer(m_pointerToCallable, std::forward<ArgTypes>(args)...);
}

template <class ReturnType, class... ArgTypes>
inline function_ref<ReturnType(ArgTypes...)>::operator bool() const noexcept
{
    return m_pointerToCallable != nullptr;
}

template <class ReturnType, class... ArgTypes>
inline void function_ref<ReturnType(ArgTypes...)>::swap(function_ref<ReturnType(ArgTypes...)>& rhs) noexcept
{
    std::swap(m_pointerToCallable, rhs.m_pointerToCallable);
    std::swap(m_functionPointer, rhs.m_functionPointer);
}

template <class ReturnType, class... ArgTypes>
inline void swap(function_ref<ReturnType(ArgTypes...)>& lhs, function_ref<ReturnType(ArgTypes...)>& rhs) noexcept
{
    lhs.swap(rhs);
}

} // namespace cxx
} // namespace iox
