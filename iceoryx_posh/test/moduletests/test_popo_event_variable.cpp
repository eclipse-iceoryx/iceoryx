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

#include "iceoryx_posh/internal/popo/building_blocks/event_variable_data.hpp"

#include "test.hpp"

using namespace ::testing;
using namespace iox::popo;

class EventVariable_test : public Test
{
  public:
    EventVariableData m_eventVarData{"Ferdinand"};
};

TEST_F(EventVariable_test, AllNotificationsAreFalseAfterConstruction)
{
    EventVariableData sut;
    for (auto& notification : sut.m_activeNotifications)
    {
        EXPECT_THAT(notification, Eq(false));
    }
}

TEST_F(EventVariable_test, CorrectProcessNameAfterConstructionWithProcessName)
{
    EXPECT_THAT(m_eventVarData.m_process.c_str(), StrEq("Ferdinand"));
}

TEST_F(EventVariable_test, AllNotificationsAreFalseAfterConstructionWithProcessName)
{
    for (auto& notification : m_eventVarData.m_activeNotifications)
    {
        EXPECT_THAT(notification, Eq(false));
    }
}

