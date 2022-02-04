// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/command_line.hpp"
#include "iceoryx_hoofs/cxx/function.hpp"
#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::cxx;

class CommandLineParser_test : public Test
{
  public:
    void SetUp()
    {
    }
    virtual void TearDown()
    {
    }
};

struct CommandLine
{
    COMMAND_LINE(CommandLine, "asdasdasd");

    OPTIONAL_VALUE(string<100>, service, {""}, 's', "service", "some description");
    REQUIRED_VALUE(string<100>, instance, 's', "instance", "some description");
    SWITCH(doStuff, 'd', "do-stuff", "do some stuff - some description");
    OPTIONAL_VALUE(uint64_t, version, 0, 'o', "bla", "sadasd");
};

TEST_F(CommandLineParser_test, asd)
{
    int argc = 0;
    char** argv = nullptr;

    CommandLine cmd(argc, argv);

    cmd.doStuff();
    cmd.service();
}
} // namespace
