// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT
#ifndef IOX_HOOFS_CXX_REQUIRES_HPP
#define IOX_HOOFS_CXX_REQUIRES_HPP

#include "iceoryx_platform/platform_correction.hpp"
#include "iox/assertions.hpp"
#include "iox/detail/deprecation_marker.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/assertions.hpp' and 'IOX_ENFORCE' instead of 'Expects'/'Ensures'.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") cxx
{

// NOLINTBEGIN(cppcoreguidelines-macro-usage) deprecated; used for legacy code
// implementing C++ Core Guideline, I.6. Prefer Expects
// see:
// https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Ri-expects
#define Expects(condition) IOX_ENFORCE(condition, "")

// implementing C++ Core Guideline, I.8. Prefer Ensures
// see:
// https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Ri-ensures
#define Ensures(condition) IOX_ENFORCE(condition, "")
// NOLINTEND(cppcoreguidelines-macro-usage)

} // namespace cxx
} // namespace iox

// clang-format on

#endif
