// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/internal/communication_channel/transmitter.hpp"
#include "iceoryx_utils/communication_channel/protocol/fifo_protocol.hpp"

#include <atomic>
#include <gmock/gmock.h>
#include <thread>

using namespace testing;
using namespace iox;
using namespace iox::units;

template <typename Transmitter_t>
class CommunicationChannelTransmitter_Test : public Test
{
  public:
    CommunicationChannelTransmitter_Test()
        : sut(&transportLayer)
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    typename Transmitter_t::TransportLayer_t transportLayer;
    Transmitter_t sut;
};

template <typename DataType>
using FiFoTestProtocol = FiFoProtocol<DataType, 100>;

using Implementations = Types<Transmitter<int, FiFoTestProtocol>>;
TYPED_TEST_CASE(CommunicationChannelTransmitter_Test, Implementations);

TYPED_TEST(CommunicationChannelTransmitter_Test, SuccessfulSend)
{
    ASSERT_THAT(this->sut.Send(441), Eq(true));
    auto result = this->transportLayer.TryReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(441));
}

TYPED_TEST(CommunicationChannelTransmitter_Test, SuccessfulMultiSend)
{
    int limit = 10;
    for (int i = 0; i < limit; ++i)
    {
        ASSERT_THAT(this->sut.Send(i), Eq(true));
    }

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->transportLayer.TryReceive();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i));
    }
}
