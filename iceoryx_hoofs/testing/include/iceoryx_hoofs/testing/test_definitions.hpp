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
#ifndef IOX_HOOFS_TESTUTILS_TEST_DEFINITIONS_HPP
#define IOX_HOOFS_TESTUTILS_TEST_DEFINITIONS_HPP

namespace iox
{
namespace testing
{
struct GTestSkipDummy
{
    template <typename T>
    GTestSkipDummy& operator<<(T&)
    {
        return *this;
    }
};

} // namespace testing
} // namespace iox

// NOLINTBEGIN(cppcoreguidelines-macro-usage) required to wrap the GTEST_SKIP macro
/// @note this can be used to enable/disable tests where additional users are needed
/// e.g.: GTEST_SKIP_FOR_ADDITIONAL_USER() << "This test requires the -DTEST_WITH_ADDITIONAL_USER=ON cmake argument";
#ifdef TEST_WITH_ADDITIONAL_USER
#define GTEST_SKIP_FOR_ADDITIONAL_USER iox::testing::GTestSkipDummy
#else
#define GTEST_SKIP_FOR_ADDITIONAL_USER() GTEST_SKIP()
#endif
// NOLINTEND(cppcoreguidelines-macro-usage)

#endif // IOX_HOOFS_TESTUTILS_TEST_DEFINITIONS_HPP
