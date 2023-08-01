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

#ifndef IOX_HOOFS_ERROR_REPORTING_LOCATION_HPP
#define IOX_HOOFS_ERROR_REPORTING_LOCATION_HPP

namespace iox
{
namespace er
{
struct SourceLocation
{
    SourceLocation(const char* file, int line, const char* function)
        : file(file)
        , line(line)
        , function(function)
    {
    }

    const char* file{nullptr};
    int line{0};
    const char* function{nullptr};
};

} // namespace er
} // namespace iox

/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage) macro is required for use of location intrinsics (__FILE__ etc.)
#define CURRENT_SOURCE_LOCATION                                                                                        \
    iox::er::SourceLocation                                                                                            \
    {                                                                                                                  \
        __FILE__, __LINE__, static_cast<const char*>(__FUNCTION__)                                                     \
    } // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
      // needed for source code location, safely wrapped in macro

#endif
