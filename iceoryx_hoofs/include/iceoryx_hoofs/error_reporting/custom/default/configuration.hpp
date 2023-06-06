// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_ERROR_REPORTING_CUSTOM_DEFAULT_CONFIGURATION_HPP
#define IOX_HOOFS_ERROR_REPORTING_CUSTOM_DEFAULT_CONFIGURATION_HPP

#include "iceoryx_hoofs/error_reporting/configuration.hpp"

namespace iox
{
namespace er
{

// Specialize to change the checks (and other options if needed) at compile time.
// this can later also be done depending on a #define to select a header
// but we should avoid to have a #define for each option.
template <>
struct ConfigurationParameters<ConfigurationTag>
{
    static constexpr bool CHECK_PRECONDITIONS{true};
    static constexpr bool CHECK_ASSUMPTIONS{true};
};

} // namespace er
} // namespace iox

#endif
