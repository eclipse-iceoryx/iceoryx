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

#include "iceoryx_introspection/introspection_types.hpp"

#include <getopt.h>

namespace iox
{
namespace client
{
namespace introspection
{
static constexpr option longOptions[] = {{"help", no_argument, nullptr, 'h'},
                                         {"version", no_argument, nullptr, 'v'},
                                         {"time", required_argument, nullptr, 't'},
                                         {"mempool", no_argument, nullptr, 0},
                                         {"port", no_argument, nullptr, 0},
                                         {"process", no_argument, nullptr, 0},
                                         {"all", no_argument, nullptr, 0},
                                         {nullptr, 0, nullptr, 0}};

static constexpr const char* shortOptions = "hvt:";

static constexpr iox::units::Duration MIN_UPDATE_PERIOD = 500_ms;
static constexpr iox::units::Duration DEFAULT_UPDATE_PERIOD = 1000_ms;
static constexpr iox::units::Duration MAX_UPDATE_PERIOD = 10000_ms;

/// @brief base class for introspection
class IntrospectionApp
{
  public:
    int updatePeriodMs = DEFAULT_UPDATE_PERIOD.milliSeconds<int>();
    bool doIntrospection = false;

    IntrospectionSelection introspectionSelection;

    /// @brief contructor to create a introspection
    /// @param[in] argc forwarding of command line arguments
    /// @param[in] argv forwarding of command line arguments
    /// @param[in] config the configuration to use
    IntrospectionApp(int argc, char* argv[]) noexcept;

    virtual ~IntrospectionApp() noexcept {};

    /// @brief interface to start the execution of the introspection
    virtual void run() noexcept = 0;

    template <typename T>
    T bounded(T input, T min, T max) noexcept
    {
        return ((input >= min) ? ((input <= max) ? input : max) : min);
    };

    /// @brief Prints help to the command line
    void printHelp() noexcept;

    void printShortInfo(const std::string& binaryName) noexcept;

    void processArgs(int argc, char** argv) noexcept;

  protected:
    enum class CmdLineArgumentParsingMode
    {
        ALL,
        ONE
    };

    /// @brief this is needed for the child classes to extend the parseCmdLineArguments function
    IntrospectionApp() noexcept;
};

} // namespace introspection
} // namespace client
} // namespace iox
