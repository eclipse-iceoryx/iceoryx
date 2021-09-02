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

#include "iceoryx_hoofs/cxx/generic_raii.hpp"

namespace iox
{
namespace cxx
{
GenericRAII::GenericRAII(const std::function<void()>& cleanupFunction) noexcept
    : GenericRAII(function_ref<void()>(), cleanupFunction)
{
}

GenericRAII::GenericRAII(const function_ref<void()>& initFunction,
                         const std::function<void()>& cleanupFunction) noexcept
    : m_cleanupFunction(cleanupFunction)
{
    if (initFunction)
    {
        initFunction();
    }
}

GenericRAII::~GenericRAII() noexcept
{
    destroy();
}

GenericRAII::GenericRAII(GenericRAII&& rhs) noexcept
{
    *this = std::move(rhs);
}

GenericRAII& GenericRAII::operator=(GenericRAII&& rhs) noexcept
{
    if (this != &rhs)
    {
        destroy();
        m_cleanupFunction = rhs.m_cleanupFunction;
        rhs.m_cleanupFunction = std::function<void()>();
    }
    return *this;
}

void GenericRAII::destroy() noexcept
{
    if (m_cleanupFunction)
    {
        m_cleanupFunction();
        m_cleanupFunction = std::function<void()>();
    }
}

} // namespace cxx
} // namespace iox
