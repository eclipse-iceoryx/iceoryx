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
#ifndef IOX_UTILS_DESIGN_PATTERN_CREATION_HPP
#define IOX_UTILS_DESIGN_PATTERN_CREATION_HPP

#include "iceoryx_utils/cxx/expected.hpp"

namespace DesignPattern
{
template <typename DerivedClass, typename ErrorType>
class Creation
{
  public:
    using CreationPattern_t = Creation<DerivedClass, ErrorType>;
    using result_t = iox::cxx::expected<DerivedClass, ErrorType>;
    using errorType_t = ErrorType;

    template <typename... Targs>
    static result_t create(Targs&&... args) noexcept
    {
        return verify(DerivedClass(std::forward<Targs>(args)...));
    }

    static result_t verify(DerivedClass&& newObject) noexcept
    {
        if (!newObject.m_isInitialized)
        {
            return iox::cxx::error<ErrorType>(newObject.m_errorValue);
        }

        return iox::cxx::success<DerivedClass>(std::move(newObject));
    }

    template <typename... Targs>
    static iox::cxx::expected<ErrorType> placementCreate(void* const memory, Targs&&... args) noexcept
    {
        auto newClass = static_cast<DerivedClass*>(memory);
        new (newClass) DerivedClass(std::forward<Targs>(args)...);

        if (!newClass->m_isInitialized)
        {
            ErrorType errorValue = newClass->m_errorValue;
            newClass->~DerivedClass();
            return iox::cxx::error<ErrorType>(errorValue);
        }

        return iox::cxx::success<>();
    }

    Creation() noexcept = default;

    Creation(Creation&& rhs) noexcept
    {
        *this = std::move(rhs);
    }

    Creation& operator=(Creation&& rhs) noexcept
    {
        if (this != &rhs)
        {
            m_isInitialized = rhs.m_isInitialized;
            m_errorValue = rhs.m_errorValue;
            rhs.m_isInitialized = false;
        }
        return *this;
    }

    Creation(const Creation& rhs) noexcept = default;
    Creation& operator=(const Creation& rhs) noexcept = default;

    bool isInitialized() const noexcept
    {
        return m_isInitialized;
    }

  protected:
    bool m_isInitialized{false};
    ErrorType m_errorValue;
};

} // namespace DesignPattern

#endif // IOX_UTILS_DESIGN_PATTERN_CREATION_HPP
