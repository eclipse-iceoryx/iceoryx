// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

extern "C" {
#include "iceoryx_binding_c/runtime.h"
}

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "testutils/roudi_gtest.hpp"

using namespace iox;
using namespace iox::runtime;

class BindingC_Runtime_test : public RouDi_GTest
{
  public:
    void SetUp()
    {
    }

    void TearDown()
    {
    }
};

TEST_F(BindingC_Runtime_test, SuccessfulRegistration)
{
    constexpr char EXPECTED_APP_NAME[iox::MAX_PROCESS_NAME_LENGTH + 1] = "chucky";
    iox_runtime_init(EXPECTED_APP_NAME);

    char actualAppName[iox::MAX_PROCESS_NAME_LENGTH + 1];
    auto nameLength = iox_runtime_get_instance_name(actualAppName, iox::MAX_PROCESS_NAME_LENGTH + 1);

    ASSERT_THAT(nameLength, Eq(strnlen(EXPECTED_APP_NAME, iox::MAX_PROCESS_NAME_LENGTH + 1)));
    EXPECT_THAT(actualAppName, StrEq(EXPECTED_APP_NAME));
}

TEST_F(BindingC_Runtime_test, AppNameLengthIsMax)
{
    std::string maxName(iox::MAX_PROCESS_NAME_LENGTH, 's');

    iox_runtime_init(maxName.c_str());

    char actualAppName[iox::MAX_PROCESS_NAME_LENGTH + 1];
    auto nameLength = iox_runtime_get_instance_name(actualAppName, iox::MAX_PROCESS_NAME_LENGTH + 1);

    ASSERT_THAT(nameLength, Eq(iox::MAX_PROCESS_NAME_LENGTH));
}

TEST_F(BindingC_Runtime_test, AppNameLengthIsOutOfLimit)
{
    std::string tooLongName(iox::MAX_PROCESS_NAME_LENGTH + 1, 's');

    EXPECT_DEATH({ iox_runtime_init(tooLongName.c_str()); }, "Application name has more than 100 characters!");
}

TEST_F(BindingC_Runtime_test, AppNameIsNullptr)
{
    EXPECT_DEATH({ iox_runtime_init(nullptr); }, "Application name is a nullptr!");
}

TEST_F(BindingC_Runtime_test, GetInstanceNameIsNullptr)
{
    constexpr char EXPECTED_APP_NAME[iox::MAX_PROCESS_NAME_LENGTH + 1] = "chucky";
    iox_runtime_init(EXPECTED_APP_NAME);

    char actualAppName[iox::MAX_PROCESS_NAME_LENGTH + 1];
    auto nameLength = iox_runtime_get_instance_name(nullptr, iox::MAX_PROCESS_NAME_LENGTH + 1);

    ASSERT_THAT(nameLength, Eq(0U));
}

TEST_F(BindingC_Runtime_test, GetInstanceNameLengthIsLessThanAppNameLength)
{
    constexpr char ACTUAL_APP_NAME[iox::MAX_PROCESS_NAME_LENGTH + 1] = "chucky";
    constexpr char EXPECTED_APP_NAME[iox::MAX_PROCESS_NAME_LENGTH + 1] = "chuck";
    iox_runtime_init(ACTUAL_APP_NAME);

    constexpr uint64_t APP_NAME_BUFFER_LENGTH{6};
    char truncatedAppName[APP_NAME_BUFFER_LENGTH];
    for (auto& c : truncatedAppName)
    {
        c = '#';
    }
    auto nameLength = iox_runtime_get_instance_name(truncatedAppName, APP_NAME_BUFFER_LENGTH);

    ASSERT_THAT(nameLength, Eq(strnlen(ACTUAL_APP_NAME, iox::MAX_PROCESS_NAME_LENGTH + 1)));
    EXPECT_THAT(truncatedAppName, StrEq(EXPECTED_APP_NAME));
}
