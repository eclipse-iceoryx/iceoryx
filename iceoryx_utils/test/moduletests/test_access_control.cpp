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
#include "iceoryx_utils/internal/posix_wrapper/access_control.hpp"

#include <pwd.h>
#include <stdlib.h>
#include <sys/stat.h>

using namespace ::testing;
using namespace iox::posix;

constexpr const char* TestFileName = "/tmp/AclTestFile.tmp";

class AccessController_test : public Test
{
  public:
    AccessController_test()
    {
    }

    void SetUp()
    {
        internal::CaptureStderr();
        m_fileStream = fopen(TestFileName, "w");
        m_fileDescriptor = fileno(m_fileStream);
    }

    void TearDown()
    {
        fclose(m_fileStream);
        std::remove(TestFileName);

        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    ~AccessController_test()
    {
    }

    iox::posix::AccessController m_accessController;
    FILE* m_fileStream;
    int m_fileDescriptor;
};

TEST_F(AccessController_test, writeStandardPermissions)
{
    // should fail beacuse no access rights have been specified yet
    bool result = m_accessController.writePermissionsToFile(m_fileDescriptor);
    EXPECT_FALSE(result);

    m_accessController.addPermissionEntry(AccessController::Category::USER, AccessController::Permission::READWRITE);

    // should fail because group and others is missing
    result = m_accessController.writePermissionsToFile(m_fileDescriptor);
    EXPECT_FALSE(result);

    m_accessController.addPermissionEntry(AccessController::Category::GROUP, AccessController::Permission::NONE);
    m_accessController.addPermissionEntry(AccessController::Category::OTHERS, AccessController::Permission::READ);

    // should succeed now
    result = m_accessController.writePermissionsToFile(m_fileDescriptor);
    EXPECT_TRUE(result);

    acl_t fileACL = acl_get_fd(m_fileDescriptor);
    acl_t localACL = acl_from_text("u::rw,g::-,o::r");

    std::string fileACLText(acl_to_text(fileACL, nullptr));
    std::string localACLText(acl_to_text(localACL, nullptr));

    EXPECT_EQ(fileACLText, localACLText);

    acl_free(fileACL);
    acl_free(localACL);
}

TEST_F(AccessController_test, writeSpecialUserPermissions)
{
    bool entryAdded = m_accessController.addPermissionEntry(AccessController::Category::SPECIFIC_USER,
                                                            AccessController::Permission::READWRITE);
    // no name specified
    EXPECT_FALSE(entryAdded);

    std::string currentUserName(getpwuid(geteuid())->pw_name);

    entryAdded = m_accessController.addPermissionEntry(
        AccessController::Category::SPECIFIC_USER, AccessController::Permission::READWRITE, currentUserName);
    EXPECT_TRUE(entryAdded);

    bool entriesWrittenToFile = m_accessController.writePermissionsToFile(m_fileDescriptor);
    // standard permissions not yet defined
    EXPECT_FALSE(entriesWrittenToFile);

    // add standard permissions
    m_accessController.addPermissionEntry(AccessController::Category::USER, AccessController::Permission::READWRITE);
    m_accessController.addPermissionEntry(AccessController::Category::GROUP, AccessController::Permission::READ);
    m_accessController.addPermissionEntry(AccessController::Category::OTHERS, AccessController::Permission::NONE);

    entriesWrittenToFile = m_accessController.writePermissionsToFile(m_fileDescriptor);
    EXPECT_TRUE(entriesWrittenToFile);

    acl_t fileACL = acl_get_fd(m_fileDescriptor);
    std::string localACLShortText = "u:" + currentUserName + ":rw,u::rw,g::r,o::-,m::rw";
    acl_t localACL = acl_from_text(localACLShortText.c_str());

    std::string fileACLText(acl_to_text(fileACL, nullptr));
    std::string localACLText(acl_to_text(localACL, nullptr));

    EXPECT_EQ(fileACLText, localACLText);

    acl_free(fileACL);
    acl_free(localACL);
}

TEST_F(AccessController_test, writeSpecialGroupPermissions)
{
    bool entryAdded = m_accessController.addPermissionEntry(AccessController::Category::SPECIFIC_GROUP,
                                                            AccessController::Permission::READWRITE);
    // no name specified
    EXPECT_FALSE(entryAdded);

    std::string groupName = "root";

    entryAdded = m_accessController.addPermissionEntry(
        AccessController::Category::SPECIFIC_GROUP, AccessController::Permission::READWRITE, groupName);
    EXPECT_TRUE(entryAdded);

    bool entriesWrittenToFile = m_accessController.writePermissionsToFile(m_fileDescriptor);
    // standard permissions not yet defined
    EXPECT_FALSE(entriesWrittenToFile);

    // add standard permissions
    m_accessController.addPermissionEntry(AccessController::Category::USER, AccessController::Permission::READWRITE);
    m_accessController.addPermissionEntry(AccessController::Category::GROUP, AccessController::Permission::READ);
    m_accessController.addPermissionEntry(AccessController::Category::OTHERS, AccessController::Permission::NONE);

    entriesWrittenToFile = m_accessController.writePermissionsToFile(m_fileDescriptor);
    EXPECT_TRUE(entriesWrittenToFile);

    acl_t fileACL = acl_get_fd(m_fileDescriptor);
    acl_t localACL = acl_from_text("g:root:rw,u::rw,g::r,o::-,m::rw");

    std::string fileACLText(acl_to_text(fileACL, nullptr));
    std::string localACLText(acl_to_text(localACL, nullptr));

    EXPECT_EQ(fileACLText, localACLText);

    acl_free(fileACL);
    acl_free(localACL);
}

TEST_F(AccessController_test, writeSpecialPermissionsWithID)
{
    std::string currentUserName(getpwuid(geteuid())->pw_name);
    uid_t currentUserId(getpwuid(geteuid())->pw_uid);
    gid_t groupId = 0; // root

    bool entryAdded = m_accessController.addPermissionEntry(
        AccessController::Category::SPECIFIC_USER, AccessController::Permission::READWRITE, currentUserId);

    EXPECT_TRUE(entryAdded);

    entryAdded = m_accessController.addPermissionEntry(
        AccessController::Category::SPECIFIC_GROUP, AccessController::Permission::READWRITE, groupId);

    EXPECT_TRUE(entryAdded);

    m_accessController.addPermissionEntry(AccessController::Category::USER, AccessController::Permission::READWRITE);
    m_accessController.addPermissionEntry(AccessController::Category::GROUP, AccessController::Permission::READ);
    m_accessController.addPermissionEntry(AccessController::Category::OTHERS, AccessController::Permission::NONE);

    bool entriesWrittenToFile = m_accessController.writePermissionsToFile(m_fileDescriptor);

    EXPECT_TRUE(entriesWrittenToFile);

    acl_t fileACL = acl_get_fd(m_fileDescriptor);
    std::string localACLShortText = "u:" + currentUserName + ":rw,u::rw,g:root:rw,g::r,o::-,m::rw";
    acl_t localACL = acl_from_text(localACLShortText.c_str());

    std::string fileACLText(acl_to_text(fileACL, nullptr));
    std::string localACLText(acl_to_text(localACL, nullptr));

    EXPECT_EQ(fileACLText, localACLText);

    acl_free(fileACL);
    acl_free(localACL);
}

TEST_F(AccessController_test, addNameInWrongPlace)
{
    std::string currentUserName(getpwuid(geteuid())->pw_name);

    // this is not allowed as the default user should not be named explicitly
    m_accessController.addPermissionEntry(
        AccessController::Category::USER, AccessController::Permission::READWRITE, currentUserName);

    m_accessController.addPermissionEntry(AccessController::Category::GROUP, AccessController::Permission::READ);
    m_accessController.addPermissionEntry(AccessController::Category::OTHERS, AccessController::Permission::NONE);

    bool writtenToFile = m_accessController.writePermissionsToFile(m_fileDescriptor);
    EXPECT_FALSE(writtenToFile);
}

TEST_F(AccessController_test, addManyPermissions)
{
    std::string groupName = "root";

    bool entryAdded;
    for (int i = 0; i < AccessController::MaxNumOfPermissions; ++i)
    {
        entryAdded = m_accessController.addPermissionEntry(
            AccessController::Category::SPECIFIC_GROUP, AccessController::Permission::READWRITE, groupName);

        ASSERT_TRUE(entryAdded);
    }

    entryAdded = m_accessController.addPermissionEntry(
        AccessController::Category::SPECIFIC_GROUP, AccessController::Permission::READWRITE, groupName);

    EXPECT_FALSE(entryAdded);

    bool entriesWrittenToFile = m_accessController.writePermissionsToFile(m_fileDescriptor);

    // the same specific group has been entered several times
    EXPECT_FALSE(entriesWrittenToFile);
}

TEST_F(AccessController_test, addStrangeNames)
{
    bool entryAdded = m_accessController.addPermissionEntry(AccessController::Category::SPECIFIC_USER,
                                                            AccessController::Permission::READWRITE,
                                                            "VeryUnlikelyThatThisUserExistsOnThisMachine123456");
    // non-existing user name specified
    EXPECT_FALSE(entryAdded);

    entryAdded = m_accessController.addPermissionEntry(AccessController::Category::SPECIFIC_GROUP,
                                                       AccessController::Permission::READWRITE,
                                                       "NeverEverEverSuchAGroupNameExisted");
    // non-existing group name specified
    EXPECT_FALSE(entryAdded);
}
