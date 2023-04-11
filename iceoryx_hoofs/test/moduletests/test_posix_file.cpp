// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/posix_wrapper/thread.hpp"
#include "iceoryx_hoofs/testing/barrier.hpp"
#include "iceoryx_platform/platform_settings.hpp"
#include "iox/duration.hpp"
#include "iox/file.hpp"
#include "iox/file_path.hpp"
#include "iox/path.hpp"
#include "test.hpp"

#include <array>
#include <thread>

namespace
{
using namespace ::testing;
using namespace iox::posix;
using namespace iox::cxx;
using namespace iox;
using namespace iox::units;
using namespace iox::units::duration_literals;

struct File_test : public Test
{
    FilePath m_sut_file_path = [] {
        auto path = Path::create(platform::IOX_TEMP_DIR).expect("invalid temp dir");
        path.append("test-file").expect("invalid file name");
        return FilePath::create(path.as_string()).expect("invalid file path");
    }();

    void SetUp() override
    {
        File::remove(m_sut_file_path);
    }
};

TEST_F(File_test, CreatingFileWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f11e3aae-2e63-468f-b58a-22aeeedbd7fc");
    auto sut = FileBuilder().open_mode(OpenMode::PURGE_AND_CREATE).create(m_sut_file_path);

    EXPECT_FALSE(sut.has_error());
}

TEST_F(File_test, PurgeAndCreateRemovesExistingFile)
{
    ::testing::Test::RecordProperty("TEST_ID", "f11e3aae-2e63-468f-b58a-22aeeedbd7fc");
    {
        auto sut = FileBuilder()
                       .open_mode(OpenMode::PURGE_AND_CREATE)
                       .access_mode(posix::AccessMode::READ_WRITE)
                       .create(m_sut_file_path);
        EXPECT_FALSE(sut.has_error());

        std::array<uint8_t, 5> test_content{1, 2, 3, 4, 5};
        auto result = sut->write(test_content.data(), 5);
        ASSERT_FALSE(result.has_error());
    }

    auto sut2 = FileBuilder().open_mode(OpenMode::PURGE_AND_CREATE).create(m_sut_file_path);
    EXPECT_FALSE(sut2.has_error());
    EXPECT_THAT(sut2->get_size().value(), 0);
}

TEST_F(File_test, CreatingExclusivelyTwiceFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "e5bb9df2-a243-4c13-ade4-85915f5e1859");
    auto sut = FileBuilder().open_mode(OpenMode::EXCLUSIVE_CREATE).create(m_sut_file_path);
    ASSERT_FALSE(sut.has_error());

    auto sut2 = FileBuilder().open_mode(OpenMode::EXCLUSIVE_CREATE).create(m_sut_file_path);
    ASSERT_TRUE(sut2.has_error());
    EXPECT_THAT(sut2.get_error(), Eq(FileCreationError::AlreadyExists));
}

TEST_F(File_test, OpeningExistingFileWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b89f2e7c-bddf-4acb-abe7-af63c92a5bfe");
    auto sut = FileBuilder().open_mode(OpenMode::PURGE_AND_CREATE).create(m_sut_file_path);
    auto sut2 = FileBuilder().open_mode(OpenMode::OPEN_EXISTING).create(m_sut_file_path);

    EXPECT_FALSE(sut2.has_error());
}

TEST_F(File_test, OpeningNonExistingFileFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "ab4c647b-b567-4448-a3ee-78883a92872a");
    auto sut = FileBuilder().open_mode(OpenMode::OPEN_EXISTING).create(m_sut_file_path);

    ASSERT_TRUE(sut.has_error());
    EXPECT_THAT(sut.get_error(), Eq(FileCreationError::DoesNotExist));
}

TEST_F(File_test, OpenOrCreateCreatesNonExistingFile)
{
    ::testing::Test::RecordProperty("TEST_ID", "6ba6cb08-df89-4f6e-a591-f34fdb065381");
    auto sut = FileBuilder().open_mode(OpenMode::OPEN_OR_CREATE).create(m_sut_file_path);

    ASSERT_FALSE(sut.has_error());
}

TEST_F(File_test, OpenOrCreateOpensExistingFile)
{
    ::testing::Test::RecordProperty("TEST_ID", "2edf5a84-08d5-4d10-8b98-0b54d8920742");
    auto sut = FileBuilder().open_mode(OpenMode::PURGE_AND_CREATE).create(m_sut_file_path);
    ASSERT_FALSE(sut.has_error());

    auto sut2 = FileBuilder().open_mode(OpenMode::OPEN_OR_CREATE).create(m_sut_file_path);
    ASSERT_FALSE(sut2.has_error());
}

TEST_F(File_test, OpenFileForReadingWithInsufficientPermissionFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "58843f37-0474-49b1-9228-207391346f70");
    auto sut =
        FileBuilder().open_mode(OpenMode::PURGE_AND_CREATE).permissions(perms::owner_write).create(m_sut_file_path);
    ASSERT_FALSE(sut.has_error());

    auto sut2 =
        FileBuilder().open_mode(OpenMode::OPEN_EXISTING).access_mode(AccessMode::READ_ONLY).create(m_sut_file_path);
    ASSERT_TRUE(sut2.has_error());
    EXPECT_THAT(sut2.get_error(), Eq(FileCreationError::PermissionDenied));
}

TEST_F(File_test, OpenFileForReadWriteWithInsufficientPermissionFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "42525850-3c77-4661-98b1-68c0701893a5");
    auto sut =
        FileBuilder().open_mode(OpenMode::PURGE_AND_CREATE).permissions(perms::owner_read).create(m_sut_file_path);
    ASSERT_FALSE(sut.has_error());

    auto sut2 =
        FileBuilder().open_mode(OpenMode::OPEN_EXISTING).access_mode(AccessMode::READ_WRITE).create(m_sut_file_path);
    ASSERT_TRUE(sut2.has_error());
    EXPECT_THAT(sut2.get_error(), Eq(FileCreationError::PermissionDenied));
}

TEST_F(File_test, AfterCreationTheFileExists)
{
    ::testing::Test::RecordProperty("TEST_ID", "076f97da-d095-4349-abcc-0f7f28d9730f");

    EXPECT_FALSE(File::does_exist(m_sut_file_path).value());
    auto sut = FileBuilder().open_mode(OpenMode::PURGE_AND_CREATE).create(m_sut_file_path);
    EXPECT_TRUE(File::does_exist(m_sut_file_path).value());
}

TEST_F(File_test, RemoveReturnsTrueWhenFileExist)
{
    ::testing::Test::RecordProperty("TEST_ID", "64123a2d-4350-4969-80ce-d66e7433ed22");

    auto sut = FileBuilder().open_mode(OpenMode::PURGE_AND_CREATE).create(m_sut_file_path);
    EXPECT_TRUE(File::remove(m_sut_file_path).value());
}

TEST_F(File_test, RemoveReturnsFalseWhenFileDoesNotExist)
{
    ::testing::Test::RecordProperty("TEST_ID", "aaed2102-288f-4a93-a2e9-db4a6cef825e");

    EXPECT_FALSE(File::remove(m_sut_file_path).value());
}

TEST_F(File_test, ReadAndWriteToFileWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "494d9d0f-3a7f-40ba-bc92-36e445332aff");

    const std::array<uint8_t, 8> test_content{12, 14, 18, 19, 22, 90, 200, 1};
    auto sut = FileBuilder()
                   .open_mode(OpenMode::PURGE_AND_CREATE)
                   .access_mode(posix::AccessMode::READ_WRITE)
                   .create(m_sut_file_path);
    ASSERT_FALSE(sut.has_error());

    auto result = sut->write(test_content.data(), test_content.size());
    ASSERT_FALSE(result.has_error());
    EXPECT_THAT(*result, Eq(test_content.size()));

    std::array<uint8_t, 8> read_content{0};
    auto read = sut->read(read_content.data(), read_content.size());
    ASSERT_FALSE(read.has_error());
    EXPECT_THAT(*read, Eq(test_content.size()));
    EXPECT_THAT(read_content, Eq(test_content));
}

TEST_F(File_test, ReadingOfAWriteOnlyFileFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "4a404243-33fb-4c28-9e7b-58980f6918a3");

    auto sut = FileBuilder()
                   .open_mode(OpenMode::PURGE_AND_CREATE)
                   .access_mode(posix::AccessMode::WRITE_ONLY)
                   .create(m_sut_file_path);
    ASSERT_FALSE(sut.has_error());

    std::array<uint8_t, 8> read_content{0};
    auto read = sut->read(read_content.data(), read_content.size());
    ASSERT_TRUE(read.has_error());
    EXPECT_THAT(read.get_error(), Eq(FileReadError::NotOpenedForReading));
}

TEST_F(File_test, ReadingWithSmallerBufferSizeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "069f9752-7f1f-4da2-9015-a583f37b9e22");

    const std::array<uint8_t, 8> test_content{112, 114, 118, 119, 122, 190, 100, 101};
    auto sut = FileBuilder()
                   .open_mode(OpenMode::PURGE_AND_CREATE)
                   .access_mode(posix::AccessMode::READ_WRITE)
                   .create(m_sut_file_path);
    ASSERT_FALSE(sut.has_error());

    auto result = sut->write(test_content.data(), test_content.size());
    ASSERT_FALSE(result.has_error());
    EXPECT_THAT(*result, Eq(test_content.size()));

    std::array<uint8_t, 2> read_content{0};
    auto read = sut->read(read_content.data(), read_content.size());

    ASSERT_FALSE(read.has_error());
    EXPECT_THAT(*read, Eq(read_content.size()));
    EXPECT_THAT(read_content, Eq(std::array<uint8_t, 2>{112, 114}));
}

TEST_F(File_test, ReadingWithLargerBufferSizeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "63a19c93-f2ea-493f-b294-d3f8ae42ec27");

    const std::array<uint8_t, 2> test_content{212, 214};
    auto sut = FileBuilder()
                   .open_mode(OpenMode::PURGE_AND_CREATE)
                   .access_mode(posix::AccessMode::READ_WRITE)
                   .create(m_sut_file_path);
    ASSERT_FALSE(sut.has_error());

    auto result = sut->write(test_content.data(), test_content.size());
    ASSERT_FALSE(result.has_error());
    EXPECT_THAT(*result, Eq(test_content.size()));

    std::array<uint8_t, 4> read_content{0};
    auto read = sut->read(read_content.data(), read_content.size());

    ASSERT_FALSE(read.has_error());
    EXPECT_THAT(*read, Eq(test_content.size()));
    EXPECT_THAT(read_content, Eq(std::array<uint8_t, 4>{212, 214, 0, 0}));
}

TEST_F(File_test, ReadingWithOffsetWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "322cc4fc-56bd-42f9-9d46-47e361006371");

    const std::array<uint8_t, 8> test_content{112, 114, 118, 119, 122, 190, 100, 101};
    auto sut = FileBuilder()
                   .open_mode(OpenMode::PURGE_AND_CREATE)
                   .access_mode(posix::AccessMode::READ_WRITE)
                   .create(m_sut_file_path);
    ASSERT_FALSE(sut.has_error());

    auto result = sut->write(test_content.data(), test_content.size());
    ASSERT_FALSE(result.has_error());
    EXPECT_THAT(*result, Eq(test_content.size()));

    std::array<uint8_t, 3> read_content{0};
    auto read = sut->read_at(2, read_content.data(), read_content.size());

    ASSERT_FALSE(read.has_error());
    EXPECT_THAT(*read, Eq(read_content.size()));
    EXPECT_THAT(read_content, Eq(std::array<uint8_t, 3>{118, 119, 122}));
}

TEST_F(File_test, ReadingWithOutOfBoundsOffsetReadsNothing)
{
    ::testing::Test::RecordProperty("TEST_ID", "e5ef080e-c74d-40d0-a6c8-71541d3057da");

    const std::array<uint8_t, 4> test_content{122, 190, 100, 101};
    auto sut = FileBuilder()
                   .open_mode(OpenMode::PURGE_AND_CREATE)
                   .access_mode(posix::AccessMode::READ_WRITE)
                   .create(m_sut_file_path);
    ASSERT_FALSE(sut.has_error());

    auto result = sut->write(test_content.data(), test_content.size());
    ASSERT_FALSE(result.has_error());
    EXPECT_THAT(*result, Eq(test_content.size()));

    std::array<uint8_t, 3> read_content{0};
    auto read = sut->read_at(8, read_content.data(), read_content.size());

    ASSERT_FALSE(read.has_error());
    EXPECT_THAT(*read, Eq(0));
}
} // namespace
