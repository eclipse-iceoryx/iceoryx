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

#include "iceoryx_utils/cxx/optional.hpp"

#include <string>

namespace iox
{
namespace posix
{
/// @brief wrapper class holding the command line arguments of the current process
/// @code
///     // program has been called with ./program arg1 foo
///     ArgvInspector inspector;
///     std::string result;
///     inspector.getCmdlineArgument(2, result); // result = "foo"
/// @endcode

class ArgvInspector
{
  public:
    /// @brief C'tor of the ArgvInspector class. Tries to open the Linux /proc/self/cmdline file and extract the command
    /// line arguments of the running process.
    ArgvInspector();

    /// @brief get the command line argument at position f_argNumber and store into f_argument input parameter.
    /// @return false if the /proc filesystem could not be accessed or if the argument number exceeds the actual command
    /// line arguments
    bool getCmdlineArgument(const int f_argNumber, std::string& f_argument) const;

  private:
    cxx::optional<std::string> m_cmdline;
};
} // namespace posix
} // namespace iox

