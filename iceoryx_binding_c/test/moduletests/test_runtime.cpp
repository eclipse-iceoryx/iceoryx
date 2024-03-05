// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

extern "C" {
#include "iceoryx_binding_c/runtime.h"
}

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_posh/roudi_env/minimal_iceoryx_config.hpp"
#include "iceoryx_posh/testing/roudi_gtest.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::runtime;
using namespace iox::roudi_env;
using namespace iox::testing;

class BindingC_Runtime_test : public RouDi_GTest
{
  public:
    BindingC_Runtime_test()
        : RouDi_GTest(MinimalIceoryxConfigBuilder().create())
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }
};

TEST_F(BindingC_Runtime_test, SuccessfulRegistration)
{
    ::testing::Test::RecordProperty("TEST_ID", "335dc136-b2de-4db9-bdfc-db034012be7c");
    constexpr char EXPECTED_RUNTIME_NAME[iox::MAX_RUNTIME_NAME_LENGTH + 1] = "chucky";
    iox_runtime_init(EXPECTED_RUNTIME_NAME);

    char actualRuntimeName[iox::MAX_RUNTIME_NAME_LENGTH + 1];
    auto nameLength = iox_runtime_get_instance_name(actualRuntimeName, iox::MAX_RUNTIME_NAME_LENGTH + 1);

    ASSERT_THAT(nameLength, Eq(strnlen(EXPECTED_RUNTIME_NAME, iox::MAX_RUNTIME_NAME_LENGTH + 1)));
    EXPECT_THAT(actualRuntimeName, StrEq(EXPECTED_RUNTIME_NAME));
}

TEST_F(BindingC_Runtime_test, RuntimeNameLengthIsMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "854a471d-936e-4c98-b56e-ba8a7d83460e");

    std::string maxName(iox::MAX_RUNTIME_NAME_LENGTH, 's');
    iox_runtime_init(maxName.c_str());

    char actualRuntimeName[iox::MAX_RUNTIME_NAME_LENGTH + 1];
    auto nameLength = iox_runtime_get_instance_name(actualRuntimeName, iox::MAX_RUNTIME_NAME_LENGTH + 1);

    ASSERT_THAT(nameLength, Eq(iox::MAX_RUNTIME_NAME_LENGTH));
}

TEST_F(BindingC_Runtime_test, RuntimeNameLengthIsOutOfLimit)
{
    ::testing::Test::RecordProperty("TEST_ID", "8fd6735d-f331-4c9c-9a91-3f06d3856d15");
    std::string tooLongName(iox::MAX_RUNTIME_NAME_LENGTH + 1, 's');

    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_runtime_init(tooLongName.c_str());
            ;
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(BindingC_Runtime_test, RuntimeNameIsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "eb1b76c9-5420-42a9-88b3-db2e36e332de");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_runtime_init(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(BindingC_Runtime_test, GetInstanceNameIsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "84ebe40f-fcc8-4143-8330-b5178d4569a1");
    constexpr char EXPECTED_RUNTIME_NAME[iox::MAX_RUNTIME_NAME_LENGTH + 1] = "chucky";
    iox_runtime_init(EXPECTED_RUNTIME_NAME);

    auto nameLength = iox_runtime_get_instance_name(nullptr, iox::MAX_RUNTIME_NAME_LENGTH + 1);

    ASSERT_THAT(nameLength, Eq(0U));
}

TEST_F(BindingC_Runtime_test, GetInstanceNameLengthIsLessThanRuntimeNameLength)
{
    ::testing::Test::RecordProperty("TEST_ID", "82d0e0e8-ae68-43d1-b684-dea9e7cf3582");
    constexpr char ACTUAL_RUNTIME_NAME[iox::MAX_RUNTIME_NAME_LENGTH + 1] = "chucky";
    constexpr char EXPECTED_RUNTIME_NAME[iox::MAX_RUNTIME_NAME_LENGTH + 1] = "chuck";
    iox_runtime_init(ACTUAL_RUNTIME_NAME);

    constexpr uint64_t RUNTIME_NAME_BUFFER_LENGTH{6};
    char truncatedRuntimeName[RUNTIME_NAME_BUFFER_LENGTH];
    for (auto& c : truncatedRuntimeName)
    {
        c = '#';
    }
    auto nameLength = iox_runtime_get_instance_name(truncatedRuntimeName, RUNTIME_NAME_BUFFER_LENGTH);

    ASSERT_THAT(nameLength, Eq(strnlen(ACTUAL_RUNTIME_NAME, iox::MAX_RUNTIME_NAME_LENGTH + 1)));
    EXPECT_THAT(truncatedRuntimeName, StrEq(EXPECTED_RUNTIME_NAME));
}

} // namespace
