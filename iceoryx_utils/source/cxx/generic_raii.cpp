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

#include "iceoryx_utils/cxx/generic_raii.hpp"

namespace iox
{
namespace cxx
{
GenericRAII::GenericRAII(const std::function<void()> initFunction, const std::function<void()> cleanupFunction) noexcept
    : m_cleanupFunction(cleanupFunction)
{
    initFunction();
}

GenericRAII::~GenericRAII() noexcept
{
    if (m_cleanupFunction)
    {
        m_cleanupFunction();
    }
}

GenericRAII::GenericRAII(GenericRAII&& rhs) noexcept
{
    *this = std::move(rhs);
}

GenericRAII& GenericRAII::operator=(GenericRAII&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_cleanupFunction = rhs.m_cleanupFunction;
        rhs.m_cleanupFunction = std::function<void()>();
    }
    return *this;
}

} // namespace cxx
} // namespace iox
