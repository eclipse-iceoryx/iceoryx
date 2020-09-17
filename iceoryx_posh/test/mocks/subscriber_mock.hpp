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
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

class MockSubscriberPortUser
{
  public:
    MockSubscriberPortUser() = default;
    MockSubscriberPortUser(std::nullptr_t)
    {
    }
    MOCK_METHOD1(subscribe, void(const uint64_t));
    MOCK_METHOD0(unsubscribe, void());
    MOCK_CONST_METHOD0(getSubscriptionState, iox::SubscribeState());
    MOCK_METHOD0(
        tryGetChunk,
        iox::cxx::expected<iox::cxx::optional<const iox::mepoo::ChunkHeader*>, iox::popo::ChunkReceiveError>());
    MOCK_METHOD1(releaseChunk, void(iox::mepoo::ChunkHeader*));
    MOCK_METHOD0(releaseQueuedChunks, void());
    MOCK_CONST_METHOD0(hasNewChunks, bool());
    MOCK_METHOD0(hasLostChunksSinceLastCall, bool());
    MOCK_METHOD1(setConditionVariable, bool(iox::popo::ConditionVariableData*));
    MOCK_METHOD0(unsetConditionVariable, bool());
    MOCK_METHOD0(isConditionVariableSet, bool());
};
