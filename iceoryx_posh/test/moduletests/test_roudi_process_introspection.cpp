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

class ProcessIntrospection_test : public Test
{
  public:
    using ProcessIntrospection = iox::roudi::ProcessIntrospection<MockPublisherPortUser>;
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

    std::unique_ptr<ChunkMock<Topic>> createMemoryChunkAndSend(ProcessIntrospection& introspection)
    {
        std::unique_ptr<ChunkMock<Topic>> chunk{new ChunkMock<Topic>};

        EXPECT_CALL(m_publisherPortImpl_mock, sendChunk(_)).Times(1);

        introspection.send();

        return chunk;
    }

    MockPublisherPortUser m_publisherPortImpl_mock;
    iox::mepoo::MemoryManager m_memoryManager;
    iox::capro::ServiceDescription m_serviceDescription;
    iox::popo::PublisherPortData m_publisherPortData{m_serviceDescription, "Foo", &m_memoryManager};
};

TEST_F(ProcessIntrospection_test, CTOR)
{
    ProcessIntrospection m_introspection;
    EXPECT_CALL(m_publisherPortImpl_mock, stopOffer()).Times(1);
}

TEST_F(ProcessIntrospection_test, registerPublisherPort)
{
    {
        ProcessIntrospection m_introspection;
        m_introspection.registerPublisherPort(&m_publisherPortData);
    }
    // stopOffer was called
    EXPECT_THAT(m_publisherPortData.m_offeringRequested, Eq(false));
}

TEST_F(ProcessIntrospection_test, send)
{
    {
        ProcessIntrospection m_introspection;
        m_introspection.registerPublisherPort(&m_publisherPortData);

        auto chunk = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk->sample()->m_processList.size(), Eq(0U));
    }
    // stopOffer was called
    EXPECT_THAT(m_publisherPortData.m_offeringRequested, Eq(false));
}

TEST_F(ProcessIntrospection_test, addRemoveProcess)
{
    {
        ProcessIntrospection m_introspection;
        m_introspection.registerPublisherPort(&m_publisherPortData);

        const int PID = 42;
        const char PROCESS_NAME[] = "/chuck_norris";

        // m_publisherPortImpl_mock->hasSubscribersReturn = true;

        // invalid removal doesn't cause problems
        m_introspection.removeProcess(PID);
        auto chunk1 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk1->sample()->m_processList.size(), Eq(0U));

        // a new process should be sent
        m_introspection.addProcess(PID, iox::cxx::string<100>(PROCESS_NAME));
        auto chunk2 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk2->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk2->sample()->m_processList[0].m_pid, Eq(PID));
        EXPECT_THAT(iox::cxx::string<100>(PROCESS_NAME) == chunk2->sample()->m_processList[0].m_name, Eq(true));

        // list should be empty after removal
        m_introspection.removeProcess(PID);
        auto chunk3 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk3->sample()->m_processList.size(), Eq(0U));

        // if there isn't any change, no data are deliverd
        m_introspection.send();
        EXPECT_CALL(m_publisherPortImpl_mock, sendChunk(_)).Times(0);
    }
    // stopOffer was called
    EXPECT_THAT(m_publisherPortData.m_offeringRequested, Eq(false));
}

TEST_F(ProcessIntrospection_test, thread)
{
    {
        ChunkMock<Topic> chunk;

        const int PID = 42;
        const char PROCESS_NAME[] = "/chuck_norris";

        ProcessIntrospection m_introspection;

        m_introspection.registerPublisherPort(&m_publisherPortData);

        // we use the deliverChunk call to check how often the thread calls the send method
        std::chrono::milliseconds& sendIntervalSleep =
            const_cast<std::chrono::milliseconds&>(m_introspection.m_sendIntervalSleep);
        sendIntervalSleep = std::chrono::milliseconds(10);

        m_introspection.setSendInterval(10);
        m_introspection.run();

        for (size_t i = 0; i < 3; ++i)
        {
            m_introspection.addProcess(PID, iox::ProcessName_t(PROCESS_NAME));
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            m_introspection.removeProcess(PID);
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
        // within this time, the thread should have sent the 6 updates
        m_introspection.stop();

        for (size_t i = 0; i < 3; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            m_introspection.addProcess(PID, iox::ProcessName_t(PROCESS_NAME));
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            m_introspection.removeProcess(PID);
        }
        // if the thread doesn't stop, we have 12 runs after the sleep period
        EXPECT_CALL(m_publisherPortImpl_mock, offer()).Times(1);
        EXPECT_CALL(m_publisherPortImpl_mock, sendChunk(_)).Times(AtLeast(4));
    }
    // stopOffer was called
    EXPECT_THAT(m_publisherPortData.m_offeringRequested, Eq(false));
}

TEST_F(ProcessIntrospection_test, addRemoveRunnable)
{
    {
        ProcessIntrospection m_introspection;

        m_introspection.registerPublisherPort(&m_publisherPortData);

        const int PID = 42;
        const char PROCESS_NAME[] = "/chuck_norris";
        const char RUNNABLE_1[] = "the_wrecking_crew";
        const char RUNNABLE_2[] = "the_octagon";
        const char RUNNABLE_3[] = "the_hitman";

        // invalid removal of unknown runnable of unknown process
        m_introspection.removeRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_1));
        auto chunk1 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk1->sample()->m_processList.size(), Eq(0U));

        // a new process
        m_introspection.addProcess(PID, iox::ProcessName_t(PROCESS_NAME));

        // invalid removal of unknown runnable of known process
        m_introspection.removeRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_1));
        auto chunk2 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk2->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk2->sample()->m_processList[0].m_runnables.size(), Eq(0U));

        // add a runnable
        m_introspection.addRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_1));
        auto chunk3 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk3->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk3->sample()->m_processList[0].m_runnables.size(), Eq(1U));

        // add it again, must be ignored
        m_introspection.addRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_1));
        auto chunk4 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk4->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk4->sample()->m_processList[0].m_runnables.size(), Eq(1U));

        // add some more
        m_introspection.addRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_2));
        m_introspection.addRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_3));
        auto chunk5 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk5->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk5->sample()->m_processList[0].m_runnables.size(), Eq(3U));

        // remove some runnables
        m_introspection.removeRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_1));
        m_introspection.removeRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_3));
        auto chunk6 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk6->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk6->sample()->m_processList[0].m_runnables.size(), Eq(1U));
        EXPECT_THAT(strcmp(RUNNABLE_2, chunk6->sample()->m_processList[0].m_runnables[0].c_str()), Eq(0));

        // remove last runnable list empty again
        m_introspection.removeRunnable(iox::ProcessName_t(PROCESS_NAME), iox::RunnableName_t(RUNNABLE_2));
        auto chunk7 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk7->sample()->m_processList.size(), Eq(1U));
        EXPECT_THAT(chunk7->sample()->m_processList[0].m_runnables.size(), Eq(0U));
    }
    // stopOffer was called
    EXPECT_THAT(m_publisherPortData.m_offeringRequested, Eq(false));
}
