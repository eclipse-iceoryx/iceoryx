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

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/gateway/channel.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

// ======================================== Helpers ======================================== //

using IdString = iox::cxx::string<100>;

// We do not need real channel terminals to test the base class.
struct StubbedIceoryxTerminal
{
    StubbedIceoryxTerminal(iox::capro::ServiceDescription){};
};

struct StubbedExternalTerminal
{
    StubbedExternalTerminal(IdString, IdString, IdString){};
};

using TestChannel = iox::gw::Channel<StubbedIceoryxTerminal, StubbedExternalTerminal>;

// ======================================== Fixture ======================================== //
class ChannelTest : public Test
{
  public:
    void SetUp(){};
    void TearDown(){};
};

// ======================================== Tests ======================================== //
TEST_F(ChannelTest, ReturnsEmptyOptionalIfObjectPoolExhausted)
{
    auto channel = iox::gw::Channel<StubbedIceoryxTerminal, StubbedExternalTerminal>::create({"", "", ""});
}
