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

#include "iox/file_reader.hpp"
#include "iox/assertions.hpp"
#include "iox/logging.hpp"

#include "iceoryx_platform/platform_correction.hpp"
#include "iceoryx_platform/platform_settings.hpp"

#include <iostream>
#include <string>

namespace iox
{
FileReader::FileReader(const std::string& fileName, const std::string& filePath, ErrorMode errorMode) noexcept
{
    {
        // create the full file path string in a separate scope in order to prevent to trigger the leak sanitizer in the
        // test for 'ErrorMode::Terminate'
        std::string fullFilePath = filePath.empty() ? fileName : filePath + platform::IOX_PATH_SEPARATORS[0] + fileName;
        m_fileStream.open(fullFilePath, std::fstream::in);
    }

    if (!isOpen())
    {
        switch (errorMode)
        {
        case ErrorMode::Ignore:
        {
            return;
        }
        case ErrorMode::Inform:
        {
            IOX_LOG(ERROR, "Could not open file '" << fileName << "' from path '" << filePath << "'.");
            return;
        }
        case ErrorMode::Terminate:
        {
            m_fileStream.close();
            IOX_LOG(FATAL, "Could not open file '" << fileName << "' from path '" << filePath << "'!");
            IOX_PANIC("Exiting due to file open failure!");
            return;
        }
        }
    }
}
bool FileReader::isOpen() const noexcept
{
    return m_fileStream.is_open();
}

bool FileReader::readLine(std::string& buffer) noexcept
{
    return static_cast<bool>(std::getline(m_fileStream, buffer));
}

} // namespace iox
