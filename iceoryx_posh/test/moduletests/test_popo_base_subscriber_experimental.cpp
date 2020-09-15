// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/experimental/popo/base_subscriber.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

#include "mocks/subscriber_mock.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::_;

struct DummyData{
    uint64_t val = 42;
};

template<typename T, typename port_t>
class StubbedBaseSubscriber : public iox::popo::BaseSubscriber<T, port_t>
{
public:
    StubbedBaseSubscriber(iox::capro::ServiceDescription sd) : iox::popo::BaseSubscriber<T, port_t>::BaseSubscriber(sd)
    {
    }
    uid_t uid() const noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::uid();
    }
    iox::capro::ServiceDescription getServiceDescription() const noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::getServiceDescription();
    }
    iox::cxx::expected<iox::popo::SubscriberError> subscribe() noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::subscribe();
    }
    iox::SubscribeState getSubscriptionState() const noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::getSubscriptionState();
    }
    void unsubscribe() noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::unsubscribe();
    }
    bool hasData() const noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::hasData();
    }
    iox::cxx::optional<iox::cxx::unique_ptr<T>> receive() noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::receive();
    }
    iox::cxx::optional<iox::cxx::unique_ptr<iox::mepoo::ChunkHeader>> receiveHeader() noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::receiveHeader();
    }
    void clearReceiveBuffer() noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::clearReceiveBuffer();
    }
    bool setConditionVariable(iox::popo::ConditionVariableData* const conditionVariableDataPtr) noexcept override
    {
        return iox::popo::BaseSubscriber<T, port_t>::setConditionVariable(conditionVariableDataPtr);
    }
    bool unsetConditionVariable() noexcept override
    {
        return iox::popo::BaseSubscriber<T, port_t>::unsetConditionVariable();
    }
    virtual bool hasTriggered() const noexcept override
    {
        return iox::popo::BaseSubscriber<T, port_t>::hasTriggered();
    }
    port_t& getMockedPort()
    {
        return iox::popo::BaseSubscriber<T, port_t>::m_port;
    }
    uid_t getPrivateUid()
    {
        return iox::popo::BaseSubscriber<T, port_t>::uid();
    }
    iox::capro::ServiceDescription getPrivateServiceDescription()
    {
        return iox::popo::BaseSubscriber<T, port_t>::m_serviceDescription;
    }
};

using TestBaseSubscriber = StubbedBaseSubscriber<DummyData, MockSubscriberPortUser>;

// ========================= Base Publisher Tests ========================= //

class ExperimentalBaseSubscriberTest : public Test {

public:
    ExperimentalBaseSubscriberTest()
    {

    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

protected:
    TestBaseSubscriber sut{{"", "", ""}};
};


TEST_F(ExperimentalBaseSubscriberTest, GetUidProperlyRetrievesUid)
{
    // ===== Setup ===== //
    // ===== Test ===== //
    auto uid = sut.uid();
    // ===== Verify ===== //
    EXPECT_EQ(sut.getPrivateUid(), uid);
    // ===== Cleanup ===== //
}

