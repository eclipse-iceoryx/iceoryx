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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_TOOLS_ICEORYX_INTROSPECTION_INTROSPECTION_APP_HPP
#define IOX_TOOLS_ICEORYX_INTROSPECTION_INTROSPECTION_APP_HPP

#include "iceoryx_introspection/introspection_types.hpp"
#include "iceoryx_platform/getopt.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"

#include <map>
#include <ncurses.h>
#include <vector>

namespace iox
{
namespace client
{
namespace introspection
{
static constexpr option longOptions[] = {{"help", no_argument, nullptr, 'h'},
                                         {"version", no_argument, nullptr, 'v'},
                                         {"time", required_argument, nullptr, 't'},
                                         {"domain-id", required_argument, nullptr, 'd'},
                                         {"mempool", no_argument, nullptr, 0},
                                         {"port", no_argument, nullptr, 0},
                                         {"process", no_argument, nullptr, 0},
                                         {"all", no_argument, nullptr, 0},
                                         {nullptr, 0, nullptr, 0}};

static constexpr const char* shortOptions = "hvt:d:";

static constexpr iox::units::Duration MIN_UPDATE_PERIOD = 500_ms;
static constexpr iox::units::Duration DEFAULT_UPDATE_PERIOD = 1000_ms;
static constexpr iox::units::Duration MAX_UPDATE_PERIOD = 10000_ms;

/// @brief color pairs for terminal printing
enum class ColorPairs : uint8_t
{
    redOnBlack = 1,
    whiteOnRed
};

/// @brief formatting options for terminal printing
static const std::map<PrettyOptions, uint32_t> prettyMap = {
    {PrettyOptions::title, A_BOLD | COLOR_PAIR(static_cast<uint8_t>(ColorPairs::redOnBlack))},
    {PrettyOptions::highlight, A_BOLD | A_UNDERLINE},
    {PrettyOptions::error, A_BOLD | COLOR_PAIR(static_cast<uint8_t>(ColorPairs::whiteOnRed))},
    {PrettyOptions::bold, A_BOLD},
    {PrettyOptions::normal, A_NORMAL}};


/// @brief base class for introspection
class IntrospectionApp
{
  public:
    /// @brief constructor to create a introspection
    /// @param[in] argc forwarding of command line arguments
    /// @param[in] argv forwarding of command line arguments
    IntrospectionApp(int argc, char* argv[]) noexcept;

    virtual ~IntrospectionApp() noexcept {};

    /// @brief interface to start the execution of the introspection
    virtual void run() noexcept = 0;

  protected:
    enum class CmdLineArgumentParsingMode
    {
        ALL,
        ONE
    };

    IntrospectionSelection introspectionSelection;

    bool doIntrospection = false;

    /// @brief this is needed for the child classes to extend the parseCmdLineArguments function
    IntrospectionApp() noexcept;

    void
    parseCmdLineArguments(int argc,
                          char** argv,
                          CmdLineArgumentParsingMode cmdLineParsingMode = CmdLineArgumentParsingMode::ALL) noexcept;

    void runIntrospection(const iox::units::Duration updatePeriodMs,
                          const IntrospectionSelection introspectionSelection);

  private:
    /// @brief initializes ncurses terminal
    void initTerminal();

    /// @brief Erase from courser to the end of the screen
    void clearToBottom();

    /// @brief closes ncurses terminal
    void closeTerminal();

    /// @brief prints buffered data to the terminal
    void refreshTerminal();

    /// @brief updates the first pad coordinates to display
    void updateDisplayYX();

    /// @brief waits until user input or timeout
    /// @param[in] timeoutMs timeout in milliseconds (-1 to wait forever)
    void waitForUserInput(int32_t timeoutMs);

    /// @brief Prints hint in case of wrong cmd line args
    void printShortInfo(const std::string& binaryName) noexcept;

    /// @brief prints to the terminal
    /// @param[in] str string to print
    /// @param[in] pr formatting options
    void prettyPrint(const std::string& str, const PrettyOptions pr = PrettyOptions::normal);

    /// @brief prints active process IDs and names
    void printProcessIntrospectionData(const ProcessIntrospectionFieldTopic* processIntrospectionField);

    /// @brief prints table showing current mempool usage
    void printMemPoolInfo(const MemPoolIntrospectionInfo& introspectionInfo);

    template <typename Topic>
    iox::unique_ptr<iox::popo::Subscriber<Topic>>
    createSubscriber(const iox::capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief Waits till port is subscribed
    template <typename Subscriber>
    bool waitForSubscription(Subscriber& port);

    /// @brief Prepares the publisher port data before printing
    std::vector<ComposedPublisherPortData>
    composePublisherPortData(const PortIntrospectionFieldTopic* portData,
                             const PortThroughputIntrospectionFieldTopic* throughputData);

    /// @brief Prepares the subscriber port data before printing
    std::vector<ComposedSubscriberPortData>
    composeSubscriberPortData(const PortIntrospectionFieldTopic* portData,
                              const SubscriberPortChangingIntrospectionFieldTopic* subscriberPortChangingData);

    /// @brief Print the prepared publisher and subscriber port data
    void printPortIntrospectionData(const std::vector<ComposedPublisherPortData>& publisherPortData,
                                    const std::vector<ComposedSubscriberPortData>& subscriberPortData);

    /// @brief Prints help to the command line
    void printHelp() noexcept;

    template <typename T>
    T bounded(T input, T min, T max) noexcept
    {
        return ((input >= min) ? ((input <= max) ? input : max) : min);
    }

    /// @brief Update rate of the terminal
    iox::units::Duration updatePeriodMs = DEFAULT_UPDATE_PERIOD;

    /// @bried the domain ID to connect to RouDi
    iox::DomainId domainId{iox::DEFAULT_DOMAIN_ID};

    /// @brief ncurses pad
    WINDOW* pad;

    /// @brief first pad row to show on the ncurses window
    int32_t yPad{0};

    /// @brief first pad column to show on the ncurses window
    int32_t xPad{0};
};

} // namespace introspection
} // namespace client
} // namespace iox

#endif // IOX_TOOLS_ICEORYX_INTROSPECTION_INTROSPECTION_APP_HPP
