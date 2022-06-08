// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/cxx/requires.hpp"
#include "iceoryx_hoofs/error_handling/error_handling.hpp"

#include <iostream>
#include <cstdint>

namespace iox
{
namespace cxx
{
namespace internal
{
// AXIVION Next Line  AutosarC++19_03-A3.3.1 : The function is declared with in the namespace
void Require(
    const bool condition, const int8_t* file, const int32_t line, const int8_t* function, const int8_t* conditionString) noexcept
{
    if (!condition)
    {
        std::cerr << "Condition: " << conditionString << " in " << function << " is violated. (" << file << ":" << line
                  << ")" << std::endl;
        errorHandler(HoofsError::EXPECTS_ENSURES_FAILED, ErrorLevel::FATAL);
    }
}
} // namespace internal
} // namespace cxx
} // namespace iox
