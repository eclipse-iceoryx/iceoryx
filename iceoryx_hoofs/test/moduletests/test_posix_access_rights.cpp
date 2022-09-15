// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "test.hpp"

#include <cstdlib>
#include <fstream>
#include <string>

namespace
{
using namespace ::testing;

const std::string TestFileName = "/tmp/PosixAccessRights_test.tmp";

class PosixAccessRights_test : public Test
{
  public:
    void SetUp() override
    {
        fileStream.open(TestFileName, std::fstream::out | std::fstream::trunc);
        fileStream.close();

        iox::posix::posixCall(system)(("groups > " + TestFileName).c_str())
            .failureReturnValue(-1)
            .evaluate()
            .or_else([](auto& r) {
                std::cerr << "system call failed with error: " << r.getHumanReadableErrnum();
                std::terminate();
            });

        internal::CaptureStderr();
    }

    void TearDown() override
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
        if (std::remove(TestFileName.c_str()) != 0)
        {
            std::cerr << "Failed to remove temporary file '" << TestFileName
                      << "'. You'll have to remove it by yourself.";
        }
    }

    std::fstream fileStream;
};


} // namespace
