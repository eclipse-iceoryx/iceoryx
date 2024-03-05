// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_data.hpp"
#include "iceoryx_posh/internal/roudi/introspection/process_introspection.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"
#include "mocks/publisher_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

class ProcessIntrospectionAccess : public iox::roudi::ProcessIntrospection<MockPublisherPortUser>
{
  public:
    using iox::roudi::ProcessIntrospection<MockPublisherPortUser>::send;

    iox::optional<MockPublisherPortUser>& getPublisherPort()
    {
        return this->m_publisherPort;
    }
};

class ProcessIntrospection_test : public Test
{
  public:
    using Topic = iox::roudi::ProcessIntrospectionFieldTopic;

    ProcessIntrospection_test()
    {
    }

    ~ProcessIntrospection_test()
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    ChunkMock<Topic>* createMemoryChunkAndSend(ProcessIntrospectionAccess& sut)
    {
        EXPECT_CALL(sut.getPublisherPort().value(), tryAllocateChunk(_, _, _, _))
            .WillOnce(Return(ByMove(iox::ok(m_chunk.get()->chunkHeader()))));

        bool chunkWasSent = false;
        EXPECT_CALL(sut.getPublisherPort().value(), sendChunk(_)).WillOnce(Invoke([&](iox::mepoo::ChunkHeader* const) {
            chunkWasSent = true;
        }));

        sut.send();

        return chunkWasSent ? m_chunk.get() : nullptr;
    }

    std::unique_ptr<ChunkMock<Topic>> m_chunk{new ChunkMock<Topic>()};
    MockPublisherPortUser m_mockPublisherPortUserIntrospection;
};

TEST_F(ProcessIntrospection_test, CTOR)
{
    ::testing::Test::RecordProperty("TEST_ID", "74c1c79f-3c99-406b-8ccf-9e85defbd71b");
    {
        std::unique_ptr<ProcessIntrospectionAccess> introspectionAccess{new ProcessIntrospectionAccess()};
        EXPECT_THAT(introspectionAccess->getPublisherPort().has_value(), Eq(false));
    }
}

TEST_F(ProcessIntrospection_test, registerPublisherPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "fcacaa4a-7883-43d6-850f-04b78558e45b");
    {
        std::unique_ptr<ProcessIntrospectionAccess> introspectionAccess{new ProcessIntrospectionAccess()};
        introspectionAccess->registerPublisherPort(std::move(m_mockPublisherPortUserIntrospection));
        EXPECT_CALL(introspectionAccess->getPublisherPort().value(), stopOffer()).Times(1);
    }
}

TEST_F(ProcessIntrospection_test, send)
{
    ::testing::Test::RecordProperty("TEST_ID", "7faf7880-c9be-4893-8f68-15cc77a4583c");
    {
        std::unique_ptr<ProcessIntrospectionAccess> introspectionAccess{new ProcessIntrospectionAccess()};
        introspectionAccess->registerPublisherPort(std::move(m_mockPublisherPortUserIntrospection));

        auto chunk = createMemoryChunkAndSend(*introspectionAccess);
        ASSERT_THAT(chunk, Ne(nullptr));
        EXPECT_THAT(chunk->sample()->m_processList.size(), Eq(0U));
        EXPECT_CALL(introspectionAccess->getPublisherPort().value(), stopOffer()).Times(1);
    }
}

TEST_F(ProcessIntrospection_test, addRemoveProcess)
{
    ::testing::Test::RecordProperty("TEST_ID", "50d5090f-c89e-400f-b400-313df15d4193");
    {
        std::unique_ptr<ProcessIntrospectionAccess> introspectionAccess{new ProcessIntrospectionAccess()};
        introspectionAccess->registerPublisherPort(std::move(m_mockPublisherPortUserIntrospection));

        const int PID = 42;
        const char PROCESS_NAME[] = "/chuck_norris";

        // invalid removal doesn't cause problems
        introspectionAccess->removeProcess(PID);
        auto chunk1 = createMemoryChunkAndSend(*introspectionAccess);
        ASSERT_THAT(chunk1, Ne(nullptr));
        EXPECT_THAT(chunk1->sample()->m_processList.size(), Eq(0U));

        // a new process should be sent
        introspectionAccess->addProcess(PID, iox::RuntimeName_t(PROCESS_NAME));
        auto chunk2 = createMemoryChunkAndSend(*introspectionAccess);
        ASSERT_THAT(chunk2, Ne(nullptr));
        EXPECT_THAT(chunk2->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk2->sample()->m_processList[0].m_pid, Eq(PID));
        EXPECT_THAT(iox::RuntimeName_t(PROCESS_NAME) == chunk2->sample()->m_processList[0].m_name, Eq(true));

        // list should be empty after removal
        introspectionAccess->removeProcess(PID);
        auto chunk3 = createMemoryChunkAndSend(*introspectionAccess);
        ASSERT_THAT(chunk3, Ne(nullptr));
        EXPECT_THAT(chunk3->sample()->m_processList.size(), Eq(0U));

        // if there isn't any change, no data are deliverd
        EXPECT_CALL(introspectionAccess->getPublisherPort().value(), sendChunk(_)).Times(0);
        EXPECT_CALL(introspectionAccess->getPublisherPort().value(), stopOffer()).Times(1);
        introspectionAccess->send();
    }
}

TEST_F(ProcessIntrospection_test, thread)
{
    ::testing::Test::RecordProperty("TEST_ID", "3b3419dd-cc3a-4011-bf63-e2a68ab8c20f");
    {
        const int PID = 42;
        const char PROCESS_NAME[] = "/chuck_norris";

        std::unique_ptr<ProcessIntrospectionAccess> introspectionAccess{new ProcessIntrospectionAccess()};

        introspectionAccess->registerPublisherPort(std::move(m_mockPublisherPortUserIntrospection));

        iox::expected<iox::mepoo::ChunkHeader*, iox::popo::AllocationError> tryAllocateChunkResult =
            iox::ok(m_chunk.get()->chunkHeader());
        EXPECT_CALL(introspectionAccess->getPublisherPort().value(), tryAllocateChunk(_, _, _, _))
            .WillRepeatedly(Return(tryAllocateChunkResult));
        EXPECT_CALL(introspectionAccess->getPublisherPort().value(), hasSubscribers()).WillRepeatedly(Return(true));
        EXPECT_CALL(introspectionAccess->getPublisherPort().value(), offer()).Times(1);
        EXPECT_CALL(introspectionAccess->getPublisherPort().value(), stopOffer()).WillRepeatedly(Return());
        EXPECT_CALL(introspectionAccess->getPublisherPort().value(), sendChunk(_)).Times(Between(2, 8));

        using namespace iox::units::duration_literals;
        introspectionAccess->setSendInterval(10_ms);
        introspectionAccess->run();

        for (size_t i = 0; i < 3; ++i)
        {
            introspectionAccess->addProcess(PID, iox::RuntimeName_t(PROCESS_NAME));
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            introspectionAccess->removeProcess(PID);
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }

        introspectionAccess->stop();

        for (size_t i = 0; i < 3; ++i)
        // within this time, the thread should have sent the 6 updates
        {
            introspectionAccess->stop();
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            introspectionAccess->addProcess(PID, iox::RuntimeName_t(PROCESS_NAME));
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            introspectionAccess->removeProcess(PID);
        }
    }
}

} // namespace
