// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI. All rights reserved.
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

#include "iceoryx_posh/version/version_info.hpp"
#include "iceoryx_versions.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::version;

class VersionInfo_test : public Test
{
  public:
    VersionInfo_test(){};
    virtual ~VersionInfo_test(){};
    virtual void SetUp(){};
    virtual void TearDown(){};
};

TEST_F(VersionInfo_test, SerializationWorkingOnOurVersion)
{
    ::testing::Test::RecordProperty("TEST_ID", "5d0ebc47-b5f8-4faa-bb32-9df50b071019");
    VersionInfo versionInfo1(VersionInfo::getCurrentVersion().operator iox::Serialization());
    EXPECT_TRUE(versionInfo1.isValid());
    EXPECT_TRUE(versionInfo1 == VersionInfo::getCurrentVersion());
}

TEST_F(VersionInfo_test, ComparesWorkingForOurVersion)
{
    ::testing::Test::RecordProperty("TEST_ID", "2e9f5a81-a548-422b-aa87-5964ea720d77");
    VersionInfo versionInfo1(static_cast<iox::Serialization>(VersionInfo::getCurrentVersion()));
    VersionInfo versionInfo2(static_cast<iox::Serialization>(versionInfo1));

    EXPECT_TRUE(versionInfo1.isValid());
    EXPECT_TRUE(versionInfo2.isValid());
    EXPECT_TRUE(versionInfo1 == versionInfo2);
    EXPECT_FALSE(versionInfo1 != versionInfo2);
    EXPECT_TRUE(versionInfo1.checkCompatibility(versionInfo2, CompatibilityCheckLevel::OFF));
    EXPECT_TRUE(versionInfo1.checkCompatibility(versionInfo2, CompatibilityCheckLevel::MAJOR));
    EXPECT_TRUE(versionInfo1.checkCompatibility(versionInfo2, CompatibilityCheckLevel::MINOR));
    EXPECT_TRUE(versionInfo1.checkCompatibility(versionInfo2, CompatibilityCheckLevel::PATCH));
    EXPECT_TRUE(versionInfo1.checkCompatibility(versionInfo2, CompatibilityCheckLevel::COMMIT_ID));
    EXPECT_TRUE(versionInfo1.checkCompatibility(versionInfo2, CompatibilityCheckLevel::BUILD_DATE));
}

TEST_F(VersionInfo_test, CompareUnequalVersions)
{
    ::testing::Test::RecordProperty("TEST_ID", "539cd77f-e55b-4907-9a89-3f2941b9b435");
    const int versionInfosSize = 7;
    VersionInfo versionInfos[versionInfosSize] = {{11u, 22u, 33u, 44u, "abc", "efg"},
                                                  {0u, 22u, 33u, 44u, "abc", "efg"},
                                                  {11u, 0u, 33u, 44u, "abc", "efg"},
                                                  {11u, 22u, 0u, 44u, "abc", "efg"},
                                                  {11u, 22u, 33u, 0u, "abc", "efg"},
                                                  {11u, 22u, 33u, 44u, "abc", "0"},
                                                  {11u, 22u, 33u, 44u, "0", "efg"}};

    // all versions are different
    for (int i = 0; i < versionInfosSize; i++)
    {
        for (int j = 0; j < versionInfosSize; j++)
        {
            if (i != j)
            {
                EXPECT_TRUE(versionInfos[i] != versionInfos[j]);
            }
        }
    }
}

TEST_F(VersionInfo_test, ComparesVersionsSameVersionInfo)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b716597-49a4-4dab-99a2-a9703c5871e0");
    VersionInfo versionInfo1(1u, 2u, 3u, 4u, "a", "b");
    VersionInfo versionInfo2(1u, 2u, 3u, 4u, "a", "b");

    EXPECT_TRUE(versionInfo1.checkCompatibility(versionInfo2, CompatibilityCheckLevel::OFF));
    EXPECT_TRUE(versionInfo1.checkCompatibility(versionInfo2, CompatibilityCheckLevel::MAJOR));
    EXPECT_TRUE(versionInfo1.checkCompatibility(versionInfo2, CompatibilityCheckLevel::MINOR));
    EXPECT_TRUE(versionInfo1.checkCompatibility(versionInfo2, CompatibilityCheckLevel::PATCH));
    EXPECT_TRUE(versionInfo1.checkCompatibility(versionInfo2, CompatibilityCheckLevel::COMMIT_ID));
    EXPECT_TRUE(versionInfo1.checkCompatibility(versionInfo2, CompatibilityCheckLevel::BUILD_DATE));
}

TEST_F(VersionInfo_test, ComparesVersionsDifferInMajorVersion)
{
    ::testing::Test::RecordProperty("TEST_ID", "eaec3289-a869-4590-b28f-4716c79621d9");
    VersionInfo versionInfo(1u, 2u, 3u, 4u, "a", "b");
    VersionInfo versionInfoWithUnequalMajorVersion(0u, 2u, 3u, 4u, "a", "b");

    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalMajorVersion, CompatibilityCheckLevel::OFF));
    EXPECT_FALSE(versionInfo.checkCompatibility(versionInfoWithUnequalMajorVersion, CompatibilityCheckLevel::MAJOR));
    EXPECT_FALSE(versionInfo.checkCompatibility(versionInfoWithUnequalMajorVersion, CompatibilityCheckLevel::MINOR));
    EXPECT_FALSE(versionInfo.checkCompatibility(versionInfoWithUnequalMajorVersion, CompatibilityCheckLevel::PATCH));
    EXPECT_FALSE(
        versionInfo.checkCompatibility(versionInfoWithUnequalMajorVersion, CompatibilityCheckLevel::COMMIT_ID));
    EXPECT_FALSE(
        versionInfo.checkCompatibility(versionInfoWithUnequalMajorVersion, CompatibilityCheckLevel::BUILD_DATE));
}

TEST_F(VersionInfo_test, ComparesVersionsDifferInMinorVersion)
{
    ::testing::Test::RecordProperty("TEST_ID", "d8054eb9-d9ed-487d-939d-ab9ecf3be8ad");
    VersionInfo versionInfo(1u, 2u, 3u, 4u, "a", "b");
    VersionInfo versionInfoWithUnequalMinorVersion(1u, 0u, 3u, 4u, "a", "b");

    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalMinorVersion, CompatibilityCheckLevel::OFF));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalMinorVersion, CompatibilityCheckLevel::MAJOR));
    EXPECT_FALSE(versionInfo.checkCompatibility(versionInfoWithUnequalMinorVersion, CompatibilityCheckLevel::MINOR));
    EXPECT_FALSE(versionInfo.checkCompatibility(versionInfoWithUnequalMinorVersion, CompatibilityCheckLevel::PATCH));
    EXPECT_FALSE(
        versionInfo.checkCompatibility(versionInfoWithUnequalMinorVersion, CompatibilityCheckLevel::COMMIT_ID));
    EXPECT_FALSE(
        versionInfo.checkCompatibility(versionInfoWithUnequalMinorVersion, CompatibilityCheckLevel::BUILD_DATE));
}

TEST_F(VersionInfo_test, ComparesVersionsDifferInPatchVersion)
{
    ::testing::Test::RecordProperty("TEST_ID", "f335673c-c39b-4259-a5ed-e361ed31268d");
    VersionInfo versionInfo(1u, 2u, 3u, 4u, "a", "b");
    VersionInfo versionInfoWithUnequalPatchVersion(1u, 2u, 0u, 4u, "a", "b");

    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalPatchVersion, CompatibilityCheckLevel::OFF));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalPatchVersion, CompatibilityCheckLevel::MAJOR));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalPatchVersion, CompatibilityCheckLevel::MINOR));
    EXPECT_FALSE(versionInfo.checkCompatibility(versionInfoWithUnequalPatchVersion, CompatibilityCheckLevel::PATCH));
    EXPECT_FALSE(
        versionInfo.checkCompatibility(versionInfoWithUnequalPatchVersion, CompatibilityCheckLevel::COMMIT_ID));
    EXPECT_FALSE(
        versionInfo.checkCompatibility(versionInfoWithUnequalPatchVersion, CompatibilityCheckLevel::BUILD_DATE));
}

TEST_F(VersionInfo_test, ComparesVersionsDifferInTweakVersion)
{
    ::testing::Test::RecordProperty("TEST_ID", "678c72fa-35f5-43ab-9c00-24b1c38585a1");
    VersionInfo versionInfo(1u, 2u, 3u, 4u, "a", "b");
    VersionInfo versionInfoWithUnequalTweakVersion(1u, 2u, 3u, 0u, "a", "b");

    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalTweakVersion, CompatibilityCheckLevel::OFF));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalTweakVersion, CompatibilityCheckLevel::MAJOR));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalTweakVersion, CompatibilityCheckLevel::MINOR));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalTweakVersion, CompatibilityCheckLevel::PATCH));
    EXPECT_FALSE(
        versionInfo.checkCompatibility(versionInfoWithUnequalTweakVersion, CompatibilityCheckLevel::COMMIT_ID));
    EXPECT_FALSE(
        versionInfo.checkCompatibility(versionInfoWithUnequalTweakVersion, CompatibilityCheckLevel::BUILD_DATE));
}

TEST_F(VersionInfo_test, ComparesVersionsDifferInCommitId)
{
    ::testing::Test::RecordProperty("TEST_ID", "92dcfcf1-8ba3-4d9f-a865-a18bc333aa4c");
    VersionInfo versionInfo(1u, 2u, 3u, 4u, "a", "b");
    VersionInfo versionInfoWithUnequalCommitId(1u, 2u, 3u, 4u, "a", "0");

    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalCommitId, CompatibilityCheckLevel::OFF));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalCommitId, CompatibilityCheckLevel::MAJOR));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalCommitId, CompatibilityCheckLevel::MINOR));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalCommitId, CompatibilityCheckLevel::PATCH));
    EXPECT_FALSE(versionInfo.checkCompatibility(versionInfoWithUnequalCommitId, CompatibilityCheckLevel::COMMIT_ID));
    EXPECT_FALSE(versionInfo.checkCompatibility(versionInfoWithUnequalCommitId, CompatibilityCheckLevel::BUILD_DATE));
}

TEST_F(VersionInfo_test, ComparesVersionsDifferInBuildDate)
{
    ::testing::Test::RecordProperty("TEST_ID", "53ae9050-59dc-4dfe-9c44-17ba1eb21d69");
    VersionInfo versionInfo(1u, 2u, 3u, 4u, "a", "b");
    VersionInfo versionInfoWithUnequalBuildDate(1u, 2u, 3u, 4u, "0", "b");

    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalBuildDate, CompatibilityCheckLevel::OFF));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalBuildDate, CompatibilityCheckLevel::MAJOR));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalBuildDate, CompatibilityCheckLevel::MINOR));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalBuildDate, CompatibilityCheckLevel::PATCH));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalBuildDate, CompatibilityCheckLevel::COMMIT_ID));
    EXPECT_FALSE(versionInfo.checkCompatibility(versionInfoWithUnequalBuildDate, CompatibilityCheckLevel::BUILD_DATE));
}

} // namespace
