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

#include "iceoryx_utils/internal/file_reader/file_reader.hpp"

namespace iox
{
namespace cxx
{
FileReader::FileReader(const std::string& f_fileName, const std::string& f_filePath, ErrorMode f_errorMode)
    : m_errorMode{f_errorMode}
{
    m_file = f_filePath.empty() ? f_fileName : f_filePath + '/' + f_fileName;
    m_fileStream.open(m_file, std::fstream::in);

    if (!IsOpen())
    {
        errorHandler(Error::kFILEREADER__FAILED_TO_OPEN_FILE, [=]() {
            switch (m_errorMode)
            {
            case ErrorMode::Ignore:
            {
                break;
            }
            default:
            case ErrorMode::Inform:
            {
                std::cerr << "\033[5;31m"
                          << "Could not open file '" << m_file << "'."
                          << "\033[0m" << std::endl;
                break;
            }
            case ErrorMode::Terminate:
            {
                std::cerr << "\033[5;31m"
                          << "Could not open file '" << m_file << "'. Exiting!"
                          << "\033[0m" << std::endl;
                std::terminate();
                break;
            }
            }
        });
    }
}

FileReader::~FileReader()
{
}

bool FileReader::IsOpen() const
{
    return m_fileStream.is_open();
}

bool FileReader::ReadLine(std::string& f_string)
{
    return static_cast<bool>(std::getline(m_fileStream, f_string));
}

} // namespace cxx
} // namespace iox
