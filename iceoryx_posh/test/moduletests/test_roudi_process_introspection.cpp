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

#include "test.hpp"
#include "testutils/timing_test.hpp"

using namespace ::testing;
using ::testing::Return;

#define private public
#define protected public
#include "iceoryx_posh/internal/roudi/introspection/process_introspection.hpp"
#undef private
#undef protected

#include "iceoryx_posh/internal/popo/ports/publisher_port_data.hpp"
#include "mocks/chunk_mock.hpp"
#include "mocks/publisher_mock.hpp"

class ProcessIntrospectionAccess : public iox::roudi::ProcessIntrospection<MockPublisherPortUser>
{
  public:
    void send()
    {
        iox::roudi::ProcessIntrospection<MockPublisherPortUser>::send();
    }
    iox::cxx::optional<MockPublisherPortUser>& getPublisherPort()
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

    virtual void SetUp()
    {
        internal::CaptureStdout();
        // m_publisherPortData.PublisherPortData(m_serviceDescription, "Foo", &m_memoryManager);
    }

    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStdout();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
        m_publisherPortData.~PublisherPortData();
    }

    std::unique_ptr<ChunkMock<Topic>> createMemoryChunkAndSend(ProcessIntrospectionAccess& introspectionAccess)
    {
        std::unique_ptr<ChunkMock<Topic>> chunk{new ChunkMock<Topic>};

        EXPECT_CALL(introspectionAccess.getPublisherPort().value(), tryAllocateChunk(_))
            .WillOnce(Return(ByMove(iox::cxx::success<iox::mepoo::ChunkHeader*>(chunk->chunkHeader()))));
        EXPECT_CALL(introspectionAccess.getPublisherPort().value(), sendChunk(_)).Times(1);

        introspectionAccess.send();

        return chunk;
    }

    MockPublisherPortUser m_publisherPortImpl_mock;
    iox::mepoo::MemoryManager m_memoryManager;
    iox::capro::ServiceDescription m_serviceDescription;
    iox::popo::PublisherPortData m_publisherPortData{m_serviceDescription, "Foo", &m_memoryManager};
};

TEST_F(ProcessIntrospection_test, CTOR)
{
    {
        ProcessIntrospectionAccess introspectionAccess;
        EXPECT_THAT(introspectionAccess.getPublisherPort().has_value(), Eq(false));
    }
}

TEST_F(ProcessIntrospection_test, registerPublisherPort)
{
    {
        ProcessIntrospectionAccess introspectionAccess;
        introspectionAccess.registerPublisherPort(&m_publisherPortData);
        EXPECT_CALL(introspectionAccess.getPublisherPort().value(), stopOffer()).Times(1);
    }

    EXPECT_THAT(m_publisherPortData.m_offeringRequested, Eq(false));
}

TEST_F(ProcessIntrospection_test, send)
{
    {
        ProcessIntrospectionAccess introspectionAccess;
        introspectionAccess.registerPublisherPort(&m_publisherPortData);

        auto chunk = createMemoryChunkAndSend(introspectionAccess);
        EXPECT_THAT(chunk->sample()->m_processList.size(), Eq(0U));
        EXPECT_CALL(introspectionAccess.getPublisherPort().value(), stopOffer()).Times(1);
    }
    // stopOffer was called
    EXPECT_THAT(m_publisherPortData.m_offeringRequested, Eq(false));
}

TEST_F(ProcessIntrospection_test, addRemoveProcess)
{
    {
        ProcessIntrospectionAccess introspectionAccess;
        introspectionAccess.registerPublisherPort(&m_publisherPortData);

        const int PID = 42;
        const char PROCESS_NAME[] = "/chuck_norris";

        // m_publisherPortImpl_mock->hasSubscribersReturn = true;

        // invalid removal doesn't cause problems
        introspectionAccess.removeProcess(PID);
        auto chunk1 = createMemoryChunkAndSend(introspectionAccess);
        EXPECT_THAT(chunk1->sample()->m_processList.size(), Eq(0U));

        // a new process should be sent
        introspectionAccess.addProcess(PID, iox::cxx::string<100>(PROCESS_NAME));
        auto chunk2 = createMemoryChunkAndSend(introspectionAccess);
        EXPECT_THAT(chunk2->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk2->sample()->m_processList[0].m_pid, Eq(PID));
        EXPECT_THAT(iox::cxx::string<100>(PROCESS_NAME) == chunk2->sample()->m_processList[0].m_name, Eq(true));

        // list should be empty after removal
        introspectionAccess.removeProcess(PID);
        auto chunk3 = createMemoryChunkAndSend(introspectionAccess);
        EXPECT_THAT(chunk3->sample()->m_processList.size(), Eq(0U));

        // if there isn't any change, no data are deliverd
        introspectionAccess.send();
        EXPECT_CALL(m_publisherPortImpl_mock, sendChunk(_)).Times(0);
    }
    // stopOffer was called
    EXPECT_THAT(m_publisherPortData.m_offeringRequested, Eq(false));
}

TIMING_TEST_F(ProcessIntrospection_test, thread, Repeat(5), [&] {
    {
        auto chunk = new ChunkMock<Topic>;
        auto chunk2 = chunk;

        const int PID = 42;
        const char PROCESS_NAME[] = "/chuck_norris";

        ProcessIntrospectionAccess introspectionAccess;

        introspectionAccess.registerPublisherPort(&m_publisherPortData);

        EXPECT_CALL(introspectionAccess.getPublisherPort().value(), offer()).Times(1);
        EXPECT_CALL(introspectionAccess.getPublisherPort().value(), sendChunk(_)).Times(2);
        EXPECT_CALL(introspectionAccess.getPublisherPort().value(), tryAllocateChunk(_))
            .WillOnce(Return(ByMove(iox::cxx::success<iox::mepoo::ChunkHeader*>(chunk->chunkHeader()))))
            .WillOnce(Return(ByMove(iox::cxx::success<iox::mepoo::ChunkHeader*>(chunk2->chunkHeader()))));

        // we use the deliverChunk call to check how often the thread calls the send method
        std::chrono::milliseconds& sendIntervalSleep =
            const_cast<std::chrono::milliseconds&>(introspectionAccess.m_sendIntervalSleep);
        sendIntervalSleep = std::chrono::milliseconds(100);

        introspectionAccess.setSendInterval(30);
        introspectionAccess.run();

        introspectionAccess.addProcess(PID, iox::ProcessName_t(PROCESS_NAME));
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        introspectionAccess.removeProcess(PID);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // within this time, the thread should have sent the 6 updates
        introspectionAccess.stop();
    }
    // stopOffer was called
    EXPECT_THAT(m_publisherPortData.m_offeringRequested, Eq(false));
});

TEST_F(ProcessIntrospection_test, addRemoveRunnable)
{
    {
        ProcessIntrospectionAccess introspectionAccess;

        introspectionAccess.registerPublisherPort(&m_publisherPortData);

        const int PID = 42;
        const char PROCESS_NAME[] = "/chuck_norris";
        const char RUNNABLE_1[] = "the_wrecking_crew";
        const char RUNNABLE_2[] = "the_octagon";
        const char RUNNABLE_3[] = "the_hitman";

        // invalid removal of unknown runnable of unknown process
        introspectionAccess.removeRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_1));
        auto chunk1 = createMemoryChunkAndSend(introspectionAccess);
        EXPECT_THAT(chunk1->sample()->m_processList.size(), Eq(0U));

        // a new process
        introspectionAccess.addProcess(PID, iox::ProcessName_t(PROCESS_NAME));

        // invalid removal of unknown runnable of known process
        introspectionAccess.removeRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_1));
        auto chunk2 = createMemoryChunkAndSend(introspectionAccess);
        EXPECT_THAT(chunk2->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk2->sample()->m_processList[0].m_runnables.size(), Eq(0U));

        // add a runnable
        introspectionAccess.addRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_1));
        auto chunk3 = createMemoryChunkAndSend(introspectionAccess);
        EXPECT_THAT(chunk3->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk3->sample()->m_processList[0].m_runnables.size(), Eq(1U));

        // add it again, must be ignored
        introspectionAccess.addRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_1));
        auto chunk4 = createMemoryChunkAndSend(introspectionAccess);
        EXPECT_THAT(chunk4->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk4->sample()->m_processList[0].m_runnables.size(), Eq(1U));

        // add some more
        introspectionAccess.addRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_2));
        introspectionAccess.addRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_3));
        auto chunk5 = createMemoryChunkAndSend(introspectionAccess);
        EXPECT_THAT(chunk5->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk5->sample()->m_processList[0].m_runnables.size(), Eq(3U));

        // remove some runnables
        introspectionAccess.removeRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_1));
        introspectionAccess.removeRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_3));
        auto chunk6 = createMemoryChunkAndSend(introspectionAccess);
        EXPECT_THAT(chunk6->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk6->sample()->m_processList[0].m_runnables.size(), Eq(1U));
        EXPECT_THAT(strcmp(RUNNABLE_2, chunk6->sample()->m_processList[0].m_runnables[0].c_str()), Eq(0));

        // remove last runnable list empty again
        introspectionAccess.removeRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_2));
        auto chunk7 = createMemoryChunkAndSend(introspectionAccess);
        EXPECT_THAT(chunk7->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk7->sample()->m_processList[0].m_runnables.size(), Eq(0U));
    }
    // stopOffer was called
    EXPECT_THAT(m_publisherPortData.m_offeringRequested, Eq(false));
}
