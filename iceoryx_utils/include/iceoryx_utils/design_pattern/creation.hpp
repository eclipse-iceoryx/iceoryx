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
    static result_t create(Targs&&... args) noexcept;

    static result_t verify(DerivedClass&& newObject) noexcept;

    template <typename... Targs>
    static iox::cxx::expected<ErrorType> placementCreate(void* const memory, Targs&&... args) noexcept;

    Creation() noexcept = default;
    Creation(Creation&& rhs) noexcept;

    Creation& operator=(Creation&& rhs) noexcept;
    Creation(const Creation& rhs) noexcept = default;
    Creation& operator=(const Creation& rhs) noexcept = default;

    bool isInitialized() const noexcept;

  protected:
    bool m_isInitialized{false};
    ErrorType m_errorValue;
};

} // namespace DesignPattern

#include "iceoryx_utils/internal/design_pattern/creation.inl"

#endif // IOX_UTILS_DESIGN_PATTERN_CREATION_HPP
