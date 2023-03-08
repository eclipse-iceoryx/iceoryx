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

#include "test.hpp"
#include <gtest/gtest.h>


#include "iceoryx_hoofs/error_reporting/location.hpp"
#include "iceoryx_hoofs/error_reporting/platform/default/error_handler.hpp"


namespace
{

using namespace ::testing;
using namespace iox::err;

constexpr ErrorCode CODE{73};

class DefaultHandler_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    DefaultHandler sut;
};

TEST_F(DefaultHandler_test, constructionAndDestructionWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c96199c0-3921-49a9-827a-506a8a4391f3");
}

// Can only check that it can be called, there are no observable effects.
TEST_F(DefaultHandler_test, panicDoesNothing)
{
    ::testing::Test::RecordProperty("TEST_ID", "0d7f7048-94d3-42b7-a25a-1a7b506fd835");
    sut.panic();
}

// Can only check that it can be called, there are no observable effects.
TEST_F(DefaultHandler_test, reportDoesNothing)
{
    ::testing::Test::RecordProperty("TEST_ID", "9e288318-c756-4666-b779-b944b89ffaf5");
    sut.reportError(ErrorDescriptor{CURRENT_SOURCE_LOCATION, CODE});
    sut.reportViolation(ErrorDescriptor{CURRENT_SOURCE_LOCATION, CODE});
}

} // namespace
