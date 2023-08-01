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

#ifndef IOX_DUST_MODULETESTS_TEST_CXX_COMMAND_LINE_COMMON_HPP
#define IOX_DUST_MODULETESTS_TEST_CXX_COMMAND_LINE_COMMON_HPP

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

struct CmdArgs
{
    int argc = 0;
    char** argv = nullptr;

    explicit CmdArgs(const std::vector<std::string>& arguments)
        : argc{static_cast<int>(arguments.size())}
        , argv{new char*[static_cast<uint64_t>(argc)]}
    {
        contents = std::make_unique<std::vector<std::string>>(arguments);
        for (uint64_t i = 0; i < static_cast<uint64_t>(argc); ++i)
        {
            argv[i] = const_cast<char*>((*contents)[i].data());
        }
    }

    ~CmdArgs()
    {
        delete[] argv;
    }

    std::unique_ptr<std::vector<std::string>> contents;
};

class OutBuffer
{
  public:
    OutBuffer()
    {
        std::cout.rdbuf(m_capture.rdbuf());
    }
    ~OutBuffer()
    {
        std::cout.rdbuf(m_originalOutBuffer);
    }

    void clear()
    {
        m_capture.str("");
    }

    std::string output()
    {
        return m_capture.str();
    }

  private:
    std::streambuf* m_originalOutBuffer{std::cout.rdbuf()};
    std::stringstream m_capture;
};

#endif
