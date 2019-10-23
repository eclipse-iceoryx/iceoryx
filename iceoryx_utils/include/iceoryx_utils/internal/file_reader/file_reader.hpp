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

#pragma once

#include "iceoryx_utils/error_handling/error_handling.hpp"

#include <fstream>

namespace iox
{
namespace cxx
{
/// @brief Wrapper class for file reading operations. Tries to open a file in the constructor. Error handling strategy
/// can be decided by means of the ErrorMode argument.
///
/// @code
///     cxx::FileReader reader("filename");
///     std::string str;
///     if(reader.IsOpen()) {
///         reader.ReadeLine(str);
///     }
///
///     // Terminates program execution, if file cannot be opened (or found):
///     cxx::FileReader reader("filename", "path/to/file", cxx::FileReader::ErrorMode::Terminate);
/// @endcode
class FileReader
{
  public:
    /// Error handling strategy. Ignore continues execution as if nothing happened. Inform continues, but prints an
    /// error message. Terminate causes the process to exit.
    enum class ErrorMode
    {
        Ignore,
        Inform,
        Terminate
    };

    /// Opens a file and creates a FileReader object. The file path argument is optional and is ignored if empty. The
    /// error mode is evaluated when an error occurs.
    FileReader(const std::string& f_fileName,
               const std::string& f_filePath = "",
               ErrorMode f_errorMode = ErrorMode::Inform);

    FileReader(const FileReader&) = delete;
    FileReader(FileReader&&) = delete;
    FileReader& operator=(const FileReader&) = delete;
    FileReader& operator=(FileReader&&) = delete;

    ~FileReader();

    /// Check if the associated file is open.
    bool IsOpen() const;
    /// Read one line from the file and store the result in f_string.
    bool ReadLine(std::string& f_string);

  private:
    std::fstream m_fileStream;
    std::string m_file;
    ErrorMode m_errorMode;
};

} // namespace cxx
} // namespace iox
