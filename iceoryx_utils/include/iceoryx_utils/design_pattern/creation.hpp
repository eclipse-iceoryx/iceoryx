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

#pragma once

#include "iceoryx_utils/cxx/expected.hpp"

namespace DesignPattern
{
template <typename DerivedClass, typename ErrorType>
struct Creation
{
    using result_t = iox::cxx::expected<DerivedClass, ErrorType>;

    template <typename... Targs>
    static result_t create(Targs&&... args)
    {
        DerivedClass newClass(std::forward<Targs>(args)...);

        if (!newClass.m_isInitialized)
        {
            return iox::cxx::error<ErrorType>(newClass.m_errorValue);
        }
        else
        {
            return iox::cxx::success<DerivedClass>(std::move(newClass));
        }
    }

    bool isInitialized() const
    {
        return m_isInitialized;
    }

  protected:
    bool m_isInitialized{false};
    ErrorType m_errorValue;
};
} // namespace DesignPattern
