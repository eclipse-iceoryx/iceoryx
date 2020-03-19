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

#include "iceoryx_utils/fixed_string/string100.hpp"
#include "mocks/chunk_mock.hpp"
#include "mocks/senderport_mock.hpp"

class ProcessIntrospection_test : public Test
{
  public:
    using ProcessIntrospection = iox::roudi::ProcessIntrospection<SenderPort_MOCK>;
    using Topic = iox::roudi::ProcessIntrospectionFieldTopic;

    ProcessIntrospection_test()
    {
        m_senderPortImpl_mock = m_senderPortImpl.details;
    }

    ~ProcessIntrospection_test()
    {
    }

    virtual void SetUp()
    {
        internal::CaptureStdout();
    }

    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStdout();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    std::unique_ptr<ChunkMock<Topic>> createMemoryChunkAndSend(ProcessIntrospection& introspection)
    {
        std::unique_ptr<ChunkMock<Topic>> chunk{new ChunkMock<Topic>};

        m_senderPortImpl_mock->reserveSampleReturn = chunk->chunkHeader();
        m_senderPortImpl_mock->deliverChunk = 0;

        introspection.send();

        EXPECT_THAT(m_senderPortImpl_mock->deliverChunk, Eq(1));
        m_senderPortImpl_mock->deliverChunk = 0;

        return chunk;
    }

    SenderPort_MOCK m_senderPortImpl;
    std::shared_ptr<SenderPort_MOCK::mock_t> m_senderPortImpl_mock = m_senderPortImpl.details;
};

TEST_F(ProcessIntrospection_test, CTOR)
{
    ProcessIntrospection m_introspection;
    EXPECT_THAT(m_senderPortImpl_mock->deactivate, Eq(0));
}

TEST_F(ProcessIntrospection_test, registerSenderPort)
{
    {
        ProcessIntrospection m_introspection;
        m_senderPortImpl_mock->isConnectedToMembersReturn = true;
        m_introspection.registerSenderPort(std::move(m_senderPortImpl));
    }
    EXPECT_THAT(m_senderPortImpl_mock->deactivate, Eq(1));
}

TEST_F(ProcessIntrospection_test, send)
{
    {
        ProcessIntrospection m_introspection;
        m_senderPortImpl_mock->isConnectedToMembersReturn = true;
        m_introspection.registerSenderPort(std::move(m_senderPortImpl));

        auto chunk = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk->sample()->m_processList.size(), Eq(0));
    }
    EXPECT_THAT(m_senderPortImpl_mock->deactivate, Eq(1));
}

TEST_F(ProcessIntrospection_test, addRemoveProcess)
{
    {
        ProcessIntrospection m_introspection;
        m_senderPortImpl_mock->isConnectedToMembersReturn = true;
        m_introspection.registerSenderPort(std::move(m_senderPortImpl));

        const int PID = 42;
        const char PROCESS_NAME[] = "/chuck_norris";

        m_senderPortImpl_mock->hasSubscribersReturn = true;

        // invalid removal doesn't cause problems
        m_introspection.removeProcess(PID);
        auto chunk1 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk1->sample()->m_processList.size(), Eq(0));

        // a new process should be sent
        m_introspection.addProcess(PID, iox::cxx::CString100(PROCESS_NAME));
        auto chunk2 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk2->sample()->m_processList.size(), Eq(1));
        EXPECT_THAT(chunk2->sample()->m_processList[0].m_pid, Eq(PID));
        EXPECT_THAT(strcmp(PROCESS_NAME, chunk2->sample()->m_processList[0].m_name.to_cstring()), Eq(0));

        // list should be empty after removal
        m_introspection.removeProcess(PID);
        auto chunk3 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk3->sample()->m_processList.size(), Eq(0));

        // if there isn't any change, no data are deliverd
        m_introspection.send();
        EXPECT_THAT(m_senderPortImpl_mock->deliverChunk, Eq(0));
    }
    EXPECT_THAT(m_senderPortImpl_mock->deactivate, Eq(1));
}

TEST_F(ProcessIntrospection_test, thread)
{
    {
        ChunkMock<Topic> chunk;

        const int PID = 42;
        const char PROCESS_NAME[] = "/chuck_norris";

        ProcessIntrospection m_introspection;
        m_senderPortImpl_mock->isConnectedToMembersReturn = true;

        m_introspection.registerSenderPort(std::move(m_senderPortImpl));

        m_senderPortImpl_mock->reserveSampleReturn = chunk.chunkHeader();
        // we use the deliverChunk call to check how often the thread calls the send method

        std::chrono::milliseconds& sendIntervalSleep =
            const_cast<std::chrono::milliseconds&>(m_introspection.m_sendIntervalSleep);
        sendIntervalSleep = std::chrono::milliseconds(10);

        m_introspection.setSendInterval(10);
        m_introspection.run();

        for (size_t i = 0; i < 3; ++i)
        {
            m_introspection.addProcess(PID, iox::cxx::CString100(PROCESS_NAME));
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            m_introspection.removeProcess(PID);
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
        // within this time, the thread should have sent the 6 updates
        m_introspection.stop();

        for (size_t i = 0; i < 3; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            m_introspection.addProcess(PID, iox::cxx::CString100(PROCESS_NAME));
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            m_introspection.removeProcess(PID);
        }
        // if the thread doesn't stop, we have 12 runs after the sleep period
        EXPECT_THAT(m_senderPortImpl_mock->activate, Eq(1));
        EXPECT_THAT(4 <= m_senderPortImpl_mock->deliverChunk && m_senderPortImpl_mock->deliverChunk <= 8, Eq(true));
    }
    EXPECT_THAT(m_senderPortImpl_mock->deactivate, Eq(1));
}

TEST_F(ProcessIntrospection_test, addRemoveRunnable)
{
    {
        ProcessIntrospection m_introspection;
        m_senderPortImpl_mock->isConnectedToMembersReturn = true;
        m_introspection.registerSenderPort(std::move(m_senderPortImpl));

        const int PID = 42;
        const char PROCESS_NAME[] = "/chuck_norris";
        const char RUNNABLE_1[] = "the_wrecking_crew";
        const char RUNNABLE_2[] = "the_octagon";
        const char RUNNABLE_3[] = "the_hitman";

        m_senderPortImpl_mock->hasSubscribersReturn = true;

        // invalid removal of unknown runnable of unknown process
        m_introspection.removeRunnable(iox::cxx::CString100(PROCESS_NAME), iox::cxx::CString100(RUNNABLE_1));
        auto chunk1 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk1->sample()->m_processList.size(), Eq(0));

        // a new process
        m_introspection.addProcess(PID, iox::cxx::CString100(PROCESS_NAME));

        // invalid removal of unknown runnable of known process
        m_introspection.removeRunnable(iox::cxx::CString100(PROCESS_NAME), iox::cxx::CString100(RUNNABLE_1));
        auto chunk2 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk2->sample()->m_processList.size(), Eq(1));
        EXPECT_THAT(chunk2->sample()->m_processList[0].m_runnables.size(), Eq(0));

        // add a runnable
        m_introspection.addRunnable(iox::cxx::CString100(PROCESS_NAME), iox::cxx::CString100(RUNNABLE_1));
        auto chunk3 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk3->sample()->m_processList.size(), Eq(1));
        EXPECT_THAT(chunk3->sample()->m_processList[0].m_runnables.size(), Eq(1));

        // add it again, must be ignored
        m_introspection.addRunnable(iox::cxx::CString100(PROCESS_NAME), iox::cxx::CString100(RUNNABLE_1));
        auto chunk4 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk4->sample()->m_processList.size(), Eq(1));
        EXPECT_THAT(chunk4->sample()->m_processList[0].m_runnables.size(), Eq(1));

        // add some more
        m_introspection.addRunnable(iox::cxx::CString100(PROCESS_NAME), iox::cxx::CString100(RUNNABLE_2));
        m_introspection.addRunnable(iox::cxx::CString100(PROCESS_NAME), iox::cxx::CString100(RUNNABLE_3));
        auto chunk5 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk5->sample()->m_processList.size(), Eq(1));
        EXPECT_THAT(chunk5->sample()->m_processList[0].m_runnables.size(), Eq(3));

        // remove some runnables
        m_introspection.removeRunnable(iox::cxx::CString100(PROCESS_NAME), iox::cxx::CString100(RUNNABLE_1));
        m_introspection.removeRunnable(iox::cxx::CString100(PROCESS_NAME), iox::cxx::CString100(RUNNABLE_3));
        auto chunk6 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk6->sample()->m_processList.size(), Eq(1));
        EXPECT_THAT(chunk6->sample()->m_processList[0].m_runnables.size(), Eq(1));
        EXPECT_THAT(strcmp(RUNNABLE_2, chunk6->sample()->m_processList[0].m_runnables[0].to_cstring()), Eq(0));

        // remove last runnable list empty again
        m_introspection.removeRunnable(iox::cxx::CString100(PROCESS_NAME), iox::cxx::CString100(RUNNABLE_2));
        auto chunk7 = createMemoryChunkAndSend(m_introspection);
        EXPECT_THAT(chunk7->sample()->m_processList.size(), Eq(1));
        EXPECT_THAT(chunk7->sample()->m_processList[0].m_runnables.size(), Eq(0));
    }
    EXPECT_THAT(m_senderPortImpl_mock->deactivate, Eq(1));
}
