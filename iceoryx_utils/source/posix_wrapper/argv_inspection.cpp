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

#include "iceoryx_utils/internal/posix_wrapper/argv_inspection.hpp"

#include <fstream>
#include <sstream>

namespace iox
{
namespace posix
{
ArgvInspector::ArgvInspector()
{
    std::fstream fileStream("/proc/self/cmdline", std::fstream::in);
    if (fileStream)
    {
        std::string str;
        std::getline(fileStream, str);
        m_cmdline.emplace(str);
    }
}

bool ArgvInspector::getCmdlineArgument(const int f_argNumber, std::string& f_argument) const
{
    std::size_t startPos = 0;
    std::size_t endPos = std::string::npos;

    if (!m_cmdline.has_value())
    {
        f_argument = std::string();
        return false;
    }

    for (int index = 0; index <= f_argNumber; ++index)
    {
        endPos = m_cmdline->find('\0', startPos);

        if (endPos == std::string::npos)
        {
            f_argument = std::string();
            return false;
        }

        f_argument = m_cmdline->substr(startPos, endPos - startPos);

        startPos = endPos + 1;
    }

    return true;
}
} // namespace posix
} // namespace iox
