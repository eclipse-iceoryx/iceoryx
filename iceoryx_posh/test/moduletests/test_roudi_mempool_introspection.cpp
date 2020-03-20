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

#include "mocks/chunk_mock.hpp"
#include "mocks/mepoo_memory_manager_mock.hpp"
#include "mocks/senderport_mock.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::Return;

#define private public
#define protected public

#include "iceoryx_posh/internal/roudi/introspection/mempool_introspection.hpp"

#undef private
#undef protected

#include "iceoryx_posh/internal/mepoo/segment_manager.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_utils/cxx/vector.hpp"


class SegmentMock
{
  public:
    SegmentMock() = default;
    SegmentMock(const SegmentMock&){};

    MePooMemoryManager_MOCK& getMemoryManager()
    {
        return memoryManager;
    }

    iox::posix::PosixGroup getWriterGroup() const
    {
        return iox::posix::PosixGroup::getGroupOfCurrentProcess();
    }

    iox::posix::PosixGroup getReaderGroup() const
    {
        return iox::posix::PosixGroup::getGroupOfCurrentProcess();
    }

  private:
    MePooMemoryManager_MOCK memoryManager;
};

class SegmentManagerMock
{
  public:
    iox::cxx::vector<SegmentMock, iox::MAX_SHM_SEGMENTS> m_segmentContainer;
};

class MemPoolIntrospection_test : public Test
{
  public:
    using MemPoolInfoContainer = iox::roudi::MemPoolInfoContainer;
    using MemPoolInfo = iox::mepoo::MemPoolInfo;
    using MemPoolIntrospection =
        iox::roudi::MemPoolIntrospection<MePooMemoryManager_MOCK, SegmentManagerMock, SenderPort_MOCK>;
    using Topic = iox::roudi::MemPoolIntrospectionTopic;

    MemPoolIntrospection_test()
    {
    }

    ~MemPoolIntrospection_test()
    {
    }

    virtual void SetUp()
    {
        internal::CaptureStdout();
        SegmentMock segmentMock;
        m_segmentManager_mock.m_segmentContainer.push_back(segmentMock);
    }

    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStdout();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    template <typename MemPoolInfoStruct>
    void initMemPoolInfo(int index, MemPoolInfoStruct& info)
    {
        info.m_chunkSize = index * 100 + 10;
        info.m_minFreeChunks = index * 100 + 45;
        info.m_numChunks = index * 100 + 50;
        info.m_usedChunks = index * 100 + 3;
    }

    // initializes the mempool info with a defined pattern
    void initMemPoolInfoContainer(MemPoolInfoContainer& memPoolInfoContainer)
    {
        int index = 0;
        for (auto& info : memPoolInfoContainer)
        {
            initMemPoolInfo(index, info);
            index++;
        }
    }

    // compares mempool info structs and returns true if they are equal
    template <typename MemPoolInfoType1, typename MemPoolInfoType2>
    bool compareMemPoolInfo(MemPoolInfoType1& first, MemPoolInfoType2& second)
    {
        int index = 0;
        for (auto& info : first)
        {
            if (info.m_chunkSize != second[index].m_chunkSize)
            {
                return false;
            }
            if (info.m_minFreeChunks != second[index].m_minFreeChunks)
            {
                return false;
            }
            if (info.m_numChunks != second[index].m_numChunks)
            {
                return false;
            }
            if (info.m_usedChunks != second[index].m_usedChunks)
            {
                return false;
            }
            index++;
        }

        return true;
    }

    MePooMemoryManager_MOCK m_rouDiInternalMemoryManager_mock;
    SegmentManagerMock m_segmentManager_mock;
    SenderPort_MOCK m_senderPortImpl_mock;
};

TEST_F(MemPoolIntrospection_test, CTOR)
{
    auto mock = m_senderPortImpl_mock.details;

    {
        MemPoolIntrospection m_introspection(
            m_rouDiInternalMemoryManager_mock, m_segmentManager_mock, std::move(m_senderPortImpl_mock));
    }

    EXPECT_THAT(mock->activate, Eq(1));
    EXPECT_THAT(mock->deactivate, Eq(1));
}

TEST_F(MemPoolIntrospection_test, send_noSubscribers)
{
    auto mock = m_senderPortImpl_mock.details;
    MemPoolIntrospection m_introspection(
        m_rouDiInternalMemoryManager_mock, m_segmentManager_mock, std::move(m_senderPortImpl_mock));

    MemPoolInfoContainer memPoolInfoContainer;
    initMemPoolInfoContainer(memPoolInfoContainer);

    mock->hasSubscribersReturn = false;

    m_introspection.send();

    EXPECT_THAT(mock->reserveChunk, Eq(0));
}

/// @todo test with multiple segments and also test the mempool info from RouDiInternalMemoryManager
/// @todo This test is not very useful as it is highly implementation-dependent and fails if the implementation changes.
/// Should be realized as an integration test with a roudi environment and less mocking classes instead.
TEST_F(MemPoolIntrospection_test, DISABLED_send_withSubscribers)
{
    MemPoolIntrospection m_introspection(
        m_rouDiInternalMemoryManager_mock, m_segmentManager_mock, std::move(m_senderPortImpl_mock));

    MemPoolInfoContainer memPoolInfoContainer;
    MemPoolInfo memPoolInfo{0, 0, 0, 0};
    initMemPoolInfoContainer(memPoolInfoContainer);

    EXPECT_CALL(m_segmentManager_mock.m_segmentContainer.front().getMemoryManager(), getMemPoolInfo(_))
        .WillRepeatedly(Invoke([&](uint32_t index) {
            initMemPoolInfo(index, memPoolInfo);
            return memPoolInfo;
        }));

    auto mock = m_senderPortImpl_mock.details;
    ChunkMock<Topic> chunk;
    mock->hasSubscribersReturn = true;
    mock->reserveSampleReturn = chunk.chunkHeader();

    m_introspection.send(); /// @todo expect call to MemPoolHandler::getMemPoolInfo

    EXPECT_THAT(mock->deliverChunk, Eq(1));
    EXPECT_THAT(compareMemPoolInfo(memPoolInfoContainer, chunk.sample()->m_mempoolInfo), Eq(true));
}

TEST_F(MemPoolIntrospection_test, thread)
{
    auto mock = m_senderPortImpl_mock.details;
    MemPoolIntrospection m_introspection(
        m_rouDiInternalMemoryManager_mock, m_segmentManager_mock, std::move(m_senderPortImpl_mock));

    MemPoolInfoContainer memPoolInfoContainer;
    MemPoolInfo memPoolInfo{0, 0, 0, 0};
    initMemPoolInfoContainer(memPoolInfoContainer);
    EXPECT_CALL(m_rouDiInternalMemoryManager_mock, getMemPoolInfo(_)).WillRepeatedly(Invoke([&](uint32_t index) {
        initMemPoolInfo(index, memPoolInfo);
        return memPoolInfo;
    }));

    // we use the hasSubscribers call to check how often the thread calls the send method
    mock->hasSubscribersReturn = false;

    m_introspection.setSnapshotInterval(10);
    m_introspection.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(55)); // within this time, the thread should have run 6 times
    m_introspection.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(55)); // the thread should sleep, if not, we have 12 runs
    m_introspection.terminate();

    EXPECT_THAT(4 <= mock->hasSubscribers && mock->hasSubscribers <= 8, Ge(true));
}
