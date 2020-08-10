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

#include "iceoryx_posh/version/version_info.hpp"
#include "iceoryx_versions.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::version;
using namespace iox::cxx;

namespace
{
class VersionInfoSut : public VersionInfo
{
  public:
    VersionInfoSut(uint16_t versionMajor,
                   uint16_t versionMinor,
                   uint16_t versionPatch,
                   uint16_t versionTweak,
                   const BuildDateStringType& buildDateString,
                   const CommitIdStringType& commitIdString) noexcept
        : VersionInfo(versionMajor, versionMinor, versionPatch, versionTweak, buildDateString, commitIdString){};

    VersionInfoSut(const Serialization& serial) noexcept
        : VersionInfo(serial){};

    uint16_t getVersionMajor()
    {
        return m_versionMajor;
    }
    uint16_t getVersionMinor()
    {
        return m_versionMinor;
    }
    uint16_t getVersionPatch()
    {
        return m_versionPatch;
    }
    uint16_t getVersionTweak()
    {
        return m_versionTweak;
    }

    VersionInfo::BuildDateStringType getBuildDateString()
    {
        return m_buildDateString;
    }

    VersionInfo::CommitIdStringType getCommitIdString()
    {
        return m_commitIdString;
    }
};
} // namespace

class VersionInfo_test : public Test
{
  public:
    VersionInfo_test(){};
    virtual ~VersionInfo_test(){};
    virtual void SetUp(){};
    virtual void TearDown(){};
};


TEST_F(VersionInfo_test, ValidMaxVersionTweak)
{
    VersionInfoSut versionInfoSut(65535u, 65535u, 65535u, 65535u, "abc", "efg");
    EXPECT_EQ(versionInfoSut.getVersionMajor(), 65535u);
    EXPECT_EQ(versionInfoSut.getVersionMinor(), 65535u);
    EXPECT_EQ(versionInfoSut.getVersionPatch(), 65535u);
    EXPECT_EQ(versionInfoSut.getVersionTweak(), 65535u);
    EXPECT_EQ(std::string(versionInfoSut.getBuildDateString().c_str()), "abc");
    EXPECT_EQ(std::string(versionInfoSut.getCommitIdString().c_str()), "efg");
}

TEST_F(VersionInfo_test, ValidMaxVersionFinalRelease)
{
    VersionInfoSut versionInfoSut(65535u, 65535u, 65535u, 0, "", "");
    EXPECT_EQ(versionInfoSut.getVersionMajor(), 65535u);
    EXPECT_EQ(versionInfoSut.getVersionMinor(), 65535u);
    EXPECT_EQ(versionInfoSut.getVersionPatch(), 65535u);
    EXPECT_EQ(versionInfoSut.getVersionTweak(), 0u);
}

TEST_F(VersionInfo_test, SerializationWorking)
{
    VersionInfoSut versionInfoSut1(65535u, 2u, 3u, 789u, "abc", "efg");
    VersionInfoSut versionInfoSut2(versionInfoSut1.operator iox::cxx::Serialization());
    EXPECT_EQ(versionInfoSut1.getVersionMajor(), versionInfoSut2.getVersionMajor());
    EXPECT_EQ(versionInfoSut1.getVersionMinor(), versionInfoSut2.getVersionMinor());
    EXPECT_EQ(versionInfoSut1.getVersionPatch(), versionInfoSut2.getVersionPatch());
    EXPECT_EQ(versionInfoSut1.getVersionTweak(), versionInfoSut2.getVersionTweak());
    EXPECT_EQ(std::string(versionInfoSut1.getBuildDateString().c_str()),
              std::string(versionInfoSut2.getBuildDateString().c_str()));
    EXPECT_EQ(std::string(versionInfoSut1.getCommitIdString().c_str()),
              std::string(versionInfoSut2.getCommitIdString().c_str()));
}

TEST_F(VersionInfo_test, SerializationWorkingOnOurVersion)
{
    VersionInfoSut versionInfoSut1(VersionInfoSut::getCurrentVersion().operator iox::cxx::Serialization());
    EXPECT_TRUE(versionInfoSut1 == VersionInfoSut::getCurrentVersion());
}

TEST_F(VersionInfo_test, ComparesWorkingForOurVersion)
{
    VersionInfoSut versionInfoSut1(VersionInfoSut::getCurrentVersion().operator iox::cxx::Serialization());
    VersionInfoSut versionInfoSut2(versionInfoSut1.operator iox::cxx::Serialization());

    EXPECT_TRUE(versionInfoSut1 == versionInfoSut2);
    EXPECT_FALSE(versionInfoSut1 != versionInfoSut2);
    EXPECT_TRUE(versionInfoSut1.checkCompatibility(versionInfoSut2, CompatibilityCheckLevel::OFF));
    EXPECT_TRUE(versionInfoSut1.checkCompatibility(versionInfoSut2, CompatibilityCheckLevel::MAJOR));
    EXPECT_TRUE(versionInfoSut1.checkCompatibility(versionInfoSut2, CompatibilityCheckLevel::MINOR));
    EXPECT_TRUE(versionInfoSut1.checkCompatibility(versionInfoSut2, CompatibilityCheckLevel::PATCH));
    EXPECT_TRUE(versionInfoSut1.checkCompatibility(versionInfoSut2, CompatibilityCheckLevel::COMMIT_ID));
    EXPECT_TRUE(versionInfoSut1.checkCompatibility(versionInfoSut2, CompatibilityCheckLevel::BUILD_DATE));
}

TEST_F(VersionInfo_test, CompareUnequalVersions)
{
    const int versionInfoSutsSize = 7;
    VersionInfoSut versionInfoSuts[versionInfoSutsSize] = {{11u, 22u, 33u, 44u, "abc", "efg"},
                                                           {0u, 22u, 33u, 44u, "abc", "efg"},
                                                           {11u, 0u, 33u, 44u, "abc", "efg"},
                                                           {11u, 22u, 0u, 44u, "abc", "efg"},
                                                           {11u, 22u, 33u, 0u, "abc", "efg"},
                                                           {11u, 22u, 33u, 44u, "abc", "0"},
                                                           {11u, 22u, 33u, 44u, "0", "efg"}};

    // all versions are different
    for (int i = 0; i < versionInfoSutsSize; i++)
    {
        for (int j = 0; j < versionInfoSutsSize; j++)
        {
            if (i != j)
            {
                EXPECT_TRUE(versionInfoSuts[i] != versionInfoSuts[j]);
            }
        }
    }
}

TEST_F(VersionInfo_test, ComparesVersionsSameVersionInfo)
{
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
    VersionInfo versionInfo(1u, 2u, 3u, 4u, "a", "b");
    VersionInfo versionInfoWithUnequalBuildDate(1u, 2u, 3u, 4u, "0", "b");

    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalBuildDate, CompatibilityCheckLevel::OFF));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalBuildDate, CompatibilityCheckLevel::MAJOR));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalBuildDate, CompatibilityCheckLevel::MINOR));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalBuildDate, CompatibilityCheckLevel::PATCH));
    EXPECT_TRUE(versionInfo.checkCompatibility(versionInfoWithUnequalBuildDate, CompatibilityCheckLevel::COMMIT_ID));
    EXPECT_FALSE(versionInfo.checkCompatibility(versionInfoWithUnequalBuildDate, CompatibilityCheckLevel::BUILD_DATE));
}
