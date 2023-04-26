// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_hoofs/testing/test_definitions.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/stat.hpp"
#include "iceoryx_platform/types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/mepoo_segment.hpp"
#include "iox/bump_allocator.hpp"
#include "iox/expected.hpp"
#include "test.hpp"


#include <functional>
#include <memory>

namespace
{
using namespace ::testing;
using namespace iox::mepoo;
using namespace iox::posix;

class MePooSegment_test : public Test
{
  public:
    class SharedMemoryObject_MOCKBuilder;
    struct SharedMemoryObject_MOCK
    {
        using Builder = SharedMemoryObject_MOCKBuilder;
        using createFct = std::function<void(const SharedMemory::Name_t,
                                             const uint64_t,
                                             const iox::posix::AccessMode,
                                             const iox::posix::OpenMode,
                                             const void*,
                                             const iox::access_rights)>;
        SharedMemoryObject_MOCK(const SharedMemory::Name_t& name,
                                const uint64_t memorySizeInBytes,
                                const AccessMode accessMode,
                                const OpenMode openMode,
                                const void* baseAddressHint,
                                const iox::access_rights permissions)
            : m_memorySizeInBytes(memorySizeInBytes)
            , m_baseAddressHint(const_cast<void*>(baseAddressHint))
        {
            if (createVerificator)
            {
                createVerificator(name, memorySizeInBytes, accessMode, openMode, baseAddressHint, permissions);
            }
            filehandle = creat("/tmp/roudi_segment_test", S_IRWXU);
        }

        ~SharedMemoryObject_MOCK()
        {
            remove("/tmp/roudi_segment_test");
        }

        int getFileHandle()
        {
            return filehandle;
        }

        iox::expected<uint64_t, iox::FileStatError> get_size() const
        {
            return iox::ok(m_memorySizeInBytes);
        }

        void* getBaseAddress()
        {
            return &memory[0];
        }

        uint64_t m_memorySizeInBytes{0};
        void* m_baseAddressHint{nullptr};
        static constexpr int MEM_SIZE = 100000;
        char memory[MEM_SIZE];
        int filehandle;
        static createFct createVerificator;
    };

    class SharedMemoryObject_MOCKBuilder
    {
        IOX_BUILDER_PARAMETER(SharedMemory::Name_t, name, "")

        IOX_BUILDER_PARAMETER(uint64_t, memorySizeInBytes, 0U)

        IOX_BUILDER_PARAMETER(AccessMode, accessMode, AccessMode::READ_ONLY)

        IOX_BUILDER_PARAMETER(OpenMode, openMode, OpenMode::OPEN_EXISTING)

        IOX_BUILDER_PARAMETER(iox::optional<const void*>, baseAddressHint, iox::nullopt)

        IOX_BUILDER_PARAMETER(iox::access_rights, permissions, iox::perms::none)

      public:
        iox::expected<SharedMemoryObject_MOCK, SharedMemoryObjectError> create() noexcept
        {
            return iox::ok(SharedMemoryObject_MOCK(m_name,
                                                   m_memorySizeInBytes,
                                                   m_accessMode,
                                                   m_openMode,
                                                   (m_baseAddressHint) ? *m_baseAddressHint : nullptr,
                                                   m_permissions));
        }
    };

    MePooConfig setupMepooConfig()
    {
        MePooConfig config;
        config.addMemPool({128, 100});
        return config;
    }

    static constexpr uint64_t RawMemorySize{20000};
    uint8_t m_rawMemory[RawMemorySize];
    iox::BumpAllocator m_managementAllocator{iox::BumpAllocator(m_rawMemory, RawMemorySize)};

    MePooConfig mepooConfig = setupMepooConfig();

    using SUT = MePooSegment<SharedMemoryObject_MOCK, MemoryManager>;
    std::unique_ptr<SUT> createSut()
    {
        return std::make_unique<SUT>(
            mepooConfig, m_managementAllocator, PosixGroup{"iox_roudi_test1"}, PosixGroup{"iox_roudi_test2"});
    }
};
MePooSegment_test::SharedMemoryObject_MOCK::createFct MePooSegment_test::SharedMemoryObject_MOCK::createVerificator;

TEST_F(MePooSegment_test, SharedMemoryFileHandleRightsAfterConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "4719f767-3413-46cd-8d1e-92a3ca92760e");
    GTEST_SKIP() << "@todo iox-#611 Test needs to be written";
}

TEST_F(MePooSegment_test, SharedMemoryCreationParameter)
{
    ::testing::Test::RecordProperty("TEST_ID", "0fcfefd4-3a84-43a5-9805-057a60239184");
    GTEST_SKIP_FOR_ADDITIONAL_USER() << "This test requires the -DTEST_WITH_ADDITIONAL_USER=ON cmake argument";

    MePooSegment_test::SharedMemoryObject_MOCK::createVerificator = [](const SharedMemory::Name_t f_name,
                                                                       const uint64_t,
                                                                       const iox::posix::AccessMode f_accessMode,
                                                                       const iox::posix::OpenMode openMode,
                                                                       const void*,
                                                                       const iox::access_rights) {
        EXPECT_THAT(f_name, Eq(SharedMemory::Name_t("iox_roudi_test2")));
        EXPECT_THAT(f_accessMode, Eq(iox::posix::AccessMode::READ_WRITE));
        EXPECT_THAT(openMode, Eq(iox::posix::OpenMode::PURGE_AND_CREATE));
    };
    SUT sut{mepooConfig, m_managementAllocator, PosixGroup{"iox_roudi_test1"}, PosixGroup{"iox_roudi_test2"}};
    MePooSegment_test::SharedMemoryObject_MOCK::createVerificator =
        MePooSegment_test::SharedMemoryObject_MOCK::createFct();
}

TEST_F(MePooSegment_test, GetSharedMemoryObject)
{
    ::testing::Test::RecordProperty("TEST_ID", "e1c12dd0-fd7d-4be3-918b-08d16a68c8e0");
    GTEST_SKIP_FOR_ADDITIONAL_USER() << "This test requires the -DTEST_WITH_ADDITIONAL_USER=ON cmake argument";

    uint64_t memorySizeInBytes{0};
    MePooSegment_test::SharedMemoryObject_MOCK::createVerificator = [&](const SharedMemory::Name_t,
                                                                        const uint64_t f_memorySizeInBytes,
                                                                        const iox::posix::AccessMode,
                                                                        const iox::posix::OpenMode,
                                                                        const void*,
                                                                        const iox::access_rights) {
        memorySizeInBytes = f_memorySizeInBytes;
    };
    SUT sut{mepooConfig, m_managementAllocator, PosixGroup{"iox_roudi_test1"}, PosixGroup{"iox_roudi_test2"}};
    MePooSegment_test::SharedMemoryObject_MOCK::createVerificator =
        MePooSegment_test::SharedMemoryObject_MOCK::createFct();

    EXPECT_THAT(sut.getSharedMemoryObject().get_size().expect("Failed to get SHM size"), Eq(memorySizeInBytes));
}

TEST_F(MePooSegment_test, GetReaderGroup)
{
    ::testing::Test::RecordProperty("TEST_ID", "ad3fd360-3765-45ae-8285-fe4ae60c91ae");
    GTEST_SKIP_FOR_ADDITIONAL_USER() << "This test requires the -DTEST_WITH_ADDITIONAL_USER=ON cmake argument";

    auto sut = createSut();
    EXPECT_THAT(sut->getReaderGroup(), Eq(iox::posix::PosixGroup("iox_roudi_test1")));
}

TEST_F(MePooSegment_test, GetWriterGroup)
{
    ::testing::Test::RecordProperty("TEST_ID", "3aa34489-bd46-4e77-89d6-20e82211e1a4");
    GTEST_SKIP_FOR_ADDITIONAL_USER() << "This test requires the -DTEST_WITH_ADDITIONAL_USER=ON cmake argument";

    auto sut = createSut();
    EXPECT_THAT(sut->getWriterGroup(), Eq(iox::posix::PosixGroup("iox_roudi_test2")));
}

TEST_F(MePooSegment_test, GetMemoryManager)
{
    ::testing::Test::RecordProperty("TEST_ID", "4bc4af78-4beb-42eb-aee4-0f7cffb66411");
    GTEST_SKIP_FOR_ADDITIONAL_USER() << "This test requires the -DTEST_WITH_ADDITIONAL_USER=ON cmake argument";

    auto sut = createSut();
    ASSERT_THAT(sut->getMemoryManager().getNumberOfMemPools(), Eq(1U));
    auto config = sut->getMemoryManager().getMemPoolInfo(0);
    ASSERT_THAT(config.m_numChunks, Eq(100U));

    constexpr uint32_t USER_PAYLOAD_SIZE{128U};
    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    sut->getMemoryManager()
        .getChunk(chunkSettings)
        .and_then([&](auto& chunk) { EXPECT_THAT(chunk.getChunkHeader()->userPayloadSize(), Eq(USER_PAYLOAD_SIZE)); })
        .or_else([](auto& error) { GTEST_FAIL() << "getChunk failed with: " << error; });
}

} // namespace
