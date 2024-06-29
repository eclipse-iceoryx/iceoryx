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

#ifndef IOX_HOOFS_REPORTING_ERROR_REPORTING_CONFIGURATION_HPP
#define IOX_HOOFS_REPORTING_ERROR_REPORTING_CONFIGURATION_HPP

#include <type_traits>

// ***
// * Configure active checks and other compile time parameters
// ***

namespace iox
{
namespace er
{

// tag type that can be used to override the configuration in a custom implementation
struct ConfigurationTag
{
};

// can be specialized here to change parameters at compile time
template <typename T>
struct ConfigurationParameters
{
    static_assert(std::is_same<T, ConfigurationTag>::value, "Incorrect configuration tag type");

    static constexpr bool CHECK_ASSERT{ true }; /// @todo iox-#1032 deactive for release builds
};

// used by the API to obtain the compile time parameters
using Configuration = ConfigurationParameters<ConfigurationTag>;

} // namespace er
} // namespace iox

#endif // IOX_HOOFS_REPORTING_ERROR_REPORTING_CONFIGURATION_HPP
