// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/popo/notification_info.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "test.hpp"

#include <thread>

namespace
{
using namespace iox;
using namespace iox::popo;
using namespace ::testing;

class NotificationInfo_test : public Test
{
  public:
    class NotificationOriginTest
    {
      public:
        static void callback(NotificationOriginTest* const origin)
        {
            origin->m_callbackOrigin = origin;
        }

        NotificationOriginTest* m_callbackOrigin = nullptr;
    };

    NotificationInfo_test()
    {
    }

    ~NotificationInfo_test()
    {
    }

    NotificationOriginTest m_origin;
    NotificationOriginTest m_falseOrigin;
    NotificationInfo m_sut{&m_origin, 1478U, createNotificationCallback(NotificationOriginTest::callback)};
};

TEST_F(NotificationInfo_test, defaultCTorConstructsEmptyNotificationInfo)
{
    ::testing::Test::RecordProperty("TEST_ID", "5677ccf1-4072-4600-a67c-b34c7a258444");
    int bla;
    NotificationInfo sut;

    EXPECT_EQ(sut.getNotificationId(), NotificationInfo::INVALID_ID);
    EXPECT_EQ(sut.doesOriginateFrom(&bla), false);
    EXPECT_EQ(sut(), false);
}

TEST_F(NotificationInfo_test, getNotificationIdReturnsValidNotificationId)
{
    ::testing::Test::RecordProperty("TEST_ID", "68a5240f-e9ab-4bf7-93a3-2d36f4ddc176");
    EXPECT_EQ(m_sut.getNotificationId(), 1478U);
}

TEST_F(NotificationInfo_test, doesOriginateFromStatesOriginCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "c9d362dc-cd25-42e6-907d-82da6d079061");
    EXPECT_EQ(m_sut.doesOriginateFrom(&m_origin), true);
    EXPECT_EQ(m_sut.doesOriginateFrom(&m_falseOrigin), false);
}

TEST_F(NotificationInfo_test, getOriginReturnsCorrectOriginWhenHavingCorrectType)
{
    ::testing::Test::RecordProperty("TEST_ID", "a1ad935f-9658-460a-863b-57dedac07ff8");
    EXPECT_EQ(m_sut.getOrigin<NotificationOriginTest>(), &m_origin);
}

TEST_F(NotificationInfo_test, constGetOriginReturnsCorrectOriginWhenHavingCorrectType)
{
    ::testing::Test::RecordProperty("TEST_ID", "97a222ab-0ddf-4ab6-be27-056739844e20");
    EXPECT_EQ(const_cast<const NotificationInfo&>(m_sut).getOrigin<NotificationOriginTest>(), &m_origin);
}

TEST_F(NotificationInfo_test, getOriginReturnsNullptrWithWrongType)
{
    ::testing::Test::RecordProperty("TEST_ID", "badb467b-bf64-4e43-af30-77c163e90c99");

    EXPECT_THAT(m_sut.getOrigin<int>(), Eq(nullptr));

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POPO__NOTIFICATION_INFO_TYPE_INCONSISTENCY_IN_GET_ORIGIN);
}

TEST_F(NotificationInfo_test, constGetOriginReturnsNullptrWithWrongType)
{
    ::testing::Test::RecordProperty("TEST_ID", "4fdb2bed-9928-4181-b195-e411d1b16572");

    EXPECT_THAT(const_cast<const NotificationInfo&>(m_sut).getOrigin<int>(), Eq(nullptr));

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POPO__NOTIFICATION_INFO_TYPE_INCONSISTENCY_IN_GET_ORIGIN);
}

TEST_F(NotificationInfo_test, triggerCallbackReturnsTrueAndCallsCallbackWithSettedCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "b2ed19d1-cf45-4cc0-ad92-fa652d215a17");
    EXPECT_TRUE(m_sut());
    EXPECT_EQ(m_origin.m_callbackOrigin, &m_origin);
}

TEST_F(NotificationInfo_test, triggerCallbackReturnsFalseWithUnsetCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "d015f42f-cc01-467a-b568-e957a45a97e6");
    m_sut = NotificationInfo{&m_origin, 9U, NotificationCallback<NotificationOriginTest, int>{}};
    EXPECT_FALSE(m_sut());
}

} // namespace
