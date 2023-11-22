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

#include "iox/posix_call.hpp"
#include "test.hpp"

#include <cstdlib>
#include <fstream>
#include <string>

namespace
{
using namespace ::testing;

const std::string TestFileName = "/tmp/PosixAccessRights_test.tmp";

// NOTE: This test is not related to 'PosixGroup' and 'PosixUser' but only to test whether the '/tmp' directory is
// accessible
class PosixAccessRights_test : public Test
{
  public:
    void SetUp() override
    {
        fileStream.open(TestFileName, std::fstream::out | std::fstream::trunc);
        fileStream.close();

        IOX_POSIX_CALL(system)
        (("groups > " + TestFileName).c_str()).failureReturnValue(-1).evaluate().or_else([](auto& r) {
            IOX_LOG(ERROR, "system call failed with error: " << r.getHumanReadableErrnum());
            std::terminate();
        });
    }

    void TearDown() override
    {
        if (std::remove(TestFileName.c_str()) != 0)
        {
            IOX_LOG(ERROR,
                    "Failed to remove temporary file '" << TestFileName << "'. You'll have to remove it by yourself.");
        }
    }

    std::fstream fileStream;
};


} // namespace
