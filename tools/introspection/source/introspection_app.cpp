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

#include "iceoryx_introspection/introspection_app.hpp"
#include "iceoryx_introspection/introspection_types.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_versions.hpp"

#include <chrono>
#include <deque>
#include <iomanip>
#include <poll.h>
#include <thread>

using namespace iox::client::introspection;
using namespace iox::units::duration_literals;

namespace iox
{
namespace client
{
namespace introspection
{
IntrospectionApp::IntrospectionApp(int argc, char* argv[]) noexcept
{
    if (argc < 2)
    {
        printShortInfo(argv[0]);
        exit(EXIT_FAILURE);
    }

    parseCmdLineArguments(argc, argv);
}

void IntrospectionApp::printHelp() noexcept
{
    std::cout << "Usage:\n"
                 "  introspection [OPTIONS] [SUBSCRIPTION]\n"
                 "  introspection --help\n"
                 "  introspection --version\n"
                 "\nOptions:\n"
                 "  -h, --help        Display help and exit.\n"
                 "  -t, --time <ms>   Update period (in milliseconds) for the display of introspection data\n"
                 "                    [min: "
              << MIN_UPDATE_PERIOD.milliSeconds<int>() << ", max: " << MAX_UPDATE_PERIOD.milliSeconds<int>()
              << ", default: " << DEFAULT_UPDATE_PERIOD.milliSeconds<int>()
              << "]\n"
                 "  -v, --version     Display latest official iceoryx release version and exit.\n"
                 "\nSubscription:\n"
                 "  Select which introspection data you would like to receive.\n"
                 "  --all             Subscribe to all available introspection data.\n"
                 "  --mempool         Subscribe to mempool introspection data.\n"
                 "  --port            Subscribe to port introspection data.\n"
                 "  --process         Subscribe to process introspection data.\n"
              << std::endl;
}

void IntrospectionApp::printShortInfo(const std::string& binaryName) noexcept
{
    std::cout << "Run '" << binaryName << " --help' for more information." << std::endl;
}

void IntrospectionApp::parseCmdLineArguments(int argc,
                                             char** argv,
                                             CmdLineArgumentParsingMode /*cmdLineParsingMode*/) noexcept
{
    int opt;
    int index;

    while ((opt = getopt_long(argc, argv, shortOptions, longOptions, &index)) != -1)
    {
        switch (opt)
        {
        case 'h':
            printHelp();
            exit(EXIT_SUCCESS);
            break;

        case 'v':
            std::cout << "Latest official IceOryx release version: " << ICEORYX_LATEST_RELEASE_VERSION << "\n"
                      << std::endl;
            exit(EXIT_SUCCESS);
            break;

        case 't':
        {
            /// @todo Calling milliseconds() should not be ambiguous, extend units::Duration?
            iox::units::Duration l_rate = iox::units::Duration::milliseconds(static_cast<long double>(std::atoi(optarg)));
            updatePeriodMs = bounded(l_rate, MIN_UPDATE_PERIOD, MAX_UPDATE_PERIOD);
            break;
        }

        case 0:
            if (longOptions[index].flag != 0)
                break;

            if (strcmp(longOptions[index].name, "all") == 0)
            {
                introspectionSelection.mempool = introspectionSelection.port = introspectionSelection.process = true;
                doIntrospection = true;
            }
            else if (strcmp(longOptions[index].name, "port") == 0)
            {
                introspectionSelection.port = true;
                doIntrospection = true;
            }
            else if (strcmp(longOptions[index].name, "process") == 0)
            {
                introspectionSelection.process = true;
                doIntrospection = true;
            }
            else if (strcmp(longOptions[index].name, "mempool") == 0)
            {
                introspectionSelection.mempool = true;
                doIntrospection = true;
            }

            break;

        case '?':
        default:
            printShortInfo(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (!doIntrospection)
    {
        std::cout << "Wrong usage. ";
        printShortInfo(argv[0]);
        exit(EXIT_FAILURE);
    }
}

void IntrospectionApp::initTerminal()
{
    // Set up ncurses terminal
    initscr();
    curs_set(0);
    start_color();
    init_pair(static_cast<uint8_t>(ColorPairs::redOnBlack), COLOR_RED, COLOR_BLACK);
    init_pair(static_cast<uint8_t>(ColorPairs::whiteOnRed), COLOR_WHITE, COLOR_RED);

    // The pad should be big enough to hold all introspection data
    constexpr uint32_t padLines = 200;
    constexpr uint32_t padCols = 200;
    pad = newpad(padLines, padCols);

    keypad(pad, TRUE);
    nodelay(pad, TRUE);
}

void IntrospectionApp::clearToBottom()
{
    wclrtobot(pad);
}

void IntrospectionApp::closeTerminal()
{
    endwin();
}

void IntrospectionApp::refreshTerminal()
{
    prefresh(pad, yPad, xPad, 0, 0, LINES - 1, COLS - 1);

    constexpr int32_t titleLines{0};
    wmove(pad, titleLines, 0);
}

void IntrospectionApp::updateDisplayYX()
{
    constexpr int32_t yIncrement = 1;
    constexpr int32_t xIncrement = 5;

    int32_t yMax = getmaxy(pad) - LINES;
    int32_t xMax = getmaxx(pad) - COLS;

    int ch = wgetch(pad);
    if ((ch == KEY_UP) && (yPad > 0))
    {
        yPad -= yIncrement;
    }
    else if ((ch == KEY_DOWN) && ((yPad < yMax)))
    {
        yPad += yIncrement;
    }
    else if ((ch == KEY_LEFT) && (xPad > 0))
    {
        xPad -= xIncrement;
    }
    else if ((ch == KEY_RIGHT) && (xPad < xMax))
    {
        xPad += xIncrement;
    }
    else
    {
        // Nothing to do
    }
}

void IntrospectionApp::waitForUserInput(int32_t timeoutMs)
{
    struct pollfd fileDesc;
    fileDesc.fd = STDIN_FILENO;
    fileDesc.events = POLLIN;
    constexpr size_t nFileDesc = 1;
    /// @todo Wrap kernel calls with SmartC
    int32_t eventCount = poll(&fileDesc, nFileDesc, timeoutMs);

    // Event detected
    if ((eventCount == nFileDesc) && (fileDesc.revents == POLLIN))
    {
        updateDisplayYX();
        refreshTerminal();
    }
}

void IntrospectionApp::prettyPrint(const std::string& str, const PrettyOptions pr)
{
    wattron(pad, prettyMap.find(pr)->second);
    wprintw(pad, str.c_str());
    wattroff(pad, prettyMap.find(pr)->second);
}

void IntrospectionApp::printProcessIntrospectionData(const ProcessIntrospectionFieldTopic* processIntrospectionField)
{
    constexpr int32_t pidWidth{-10};
    constexpr int32_t processWidth{-10};

    for (auto& data : processIntrospectionField->m_processList)
    {
        wprintw(pad, "PID: %*d Process: %*s\n", pidWidth, data.m_pid, processWidth, data.m_name);
    }
    wprintw(pad, "\n");
}

void IntrospectionApp::printMemPoolInfo(const MemPoolIntrospectionTopic& topic)
{
    wprintw(pad, "Segment ID: %d\n", topic.m_id);

    wprintw(pad, "Shared memory segment writer group: ");
    prettyPrint(std::string(topic.m_writerGroupName), PrettyOptions::bold);
    wprintw(pad, "\n");

    wprintw(pad, "Shared memory segment reader group: ");
    prettyPrint(std::string(topic.m_readerGroupName), PrettyOptions::bold);
    wprintw(pad, "\n\n");

    constexpr int32_t memPoolWidth{8};
    constexpr int32_t usedchunksWidth{14};
    constexpr int32_t numchunksWidth{9};
    constexpr int32_t minFreechunksWidth{9};
    constexpr int32_t chunkSizeWidth{11};
    constexpr int32_t payloadSizeWidth{13};

    wprintw(pad, "%*s |", memPoolWidth, "MemPool");
    wprintw(pad, "%*s |", usedchunksWidth, "Chunks In Use");
    wprintw(pad, "%*s |", numchunksWidth, "Total");
    wprintw(pad, "%*s |", minFreechunksWidth, "Min Free");
    wprintw(pad, "%*s |", chunkSizeWidth, "Chunk Size");
    wprintw(pad, "%*s\n", payloadSizeWidth, "Payload Size");
    wprintw(pad, "--------------------------------------------------------------------------\n");

    for (size_t i = 0; i < topic.m_mempoolInfo.size(); ++i)
    {
        auto& info = topic.m_mempoolInfo[i];
        if (info.m_numChunks > 0)
        {
            wprintw(pad, "%*d |", memPoolWidth, i + 1);
            wprintw(pad, "%*d |", usedchunksWidth, info.m_usedChunks);
            wprintw(pad, "%*d |", numchunksWidth, info.m_numChunks);
            wprintw(pad, "%*d |", minFreechunksWidth, info.m_minFreeChunks);
            wprintw(pad, "%*d |", chunkSizeWidth, info.m_chunkSize);
            wprintw(pad, "%*d\n", payloadSizeWidth, info.m_payloadSize);
        }
    }
    wprintw(pad, "\n");
}

void IntrospectionApp::printPortIntrospectionData(const std::vector<ComposedSenderPortData>& senderPortData,
                                                  const std::vector<ComposedReceiverPortData>& receiverPortData)
{
    constexpr int32_t serviceWidth{16};
    constexpr int32_t instanceWidth{16};
    constexpr int32_t eventWidth{21};
    constexpr int32_t processNameWidth{23};
    constexpr int32_t sampleSizeWidth{12};
    constexpr int32_t chunkSizeWidth{12};
    constexpr int32_t chunksWidth{12};
    constexpr int32_t intervalWidth{19};
    constexpr int32_t isFieldWidth{6};
    constexpr int32_t subscriptionStateWidth{14};
    constexpr int32_t fifoWidth{17};
    constexpr int32_t callbackActiveWidth{8};
    constexpr int32_t scopeWidth{12};

    prettyPrint("Sender Ports\n", PrettyOptions::bold);

    wprintw(pad, " %*s |", serviceWidth, "Service");
    wprintw(pad, " %*s |", instanceWidth, "Instance");
    wprintw(pad, " %*s |", eventWidth, "Event");
    wprintw(pad, " %*s |", processNameWidth, "Process");
    wprintw(pad, " %*s |", sampleSizeWidth, "Sample Size");
    wprintw(pad, " %*s |", chunkSizeWidth, "Chunk Size");
    wprintw(pad, " %*s |", chunksWidth, "Chunks");
    wprintw(pad, " %*s |", intervalWidth, "Last Send Interval");
    wprintw(pad, " %*s\n", isFieldWidth, "Field");

    wprintw(pad, " %*s |", serviceWidth, "");
    wprintw(pad, " %*s |", instanceWidth, "");
    wprintw(pad, " %*s |", eventWidth, "");
    wprintw(pad, " %*s |", processNameWidth, "");
    wprintw(pad, " %*s |", sampleSizeWidth, "[Byte]");
    wprintw(pad, " %*s |", chunkSizeWidth, "[Byte]");
    wprintw(pad, " %*s |", chunksWidth, "[/Minute]");
    wprintw(pad, " %*s |", intervalWidth, "[Milliseconds]");
    wprintw(pad, " %*s\n", isFieldWidth, "");

    wprintw(pad, "---------------------------------------------------------------------------------------------------");
    wprintw(pad, "---------------------------------------------------------------\n");

    bool needsLineBreak{false};
    int currentLine{0};
    auto printEntry = [&](std::uint32_t maxSize, const std::string& data) -> std::string {
        std::stringstream stream;

        constexpr int indentation{2};
        constexpr char indentationString[indentation + 1] = "  ";

        auto stringSize = data.size();
        if (currentLine == 0)
        {
            stream << std::left << std::setw(maxSize) << data.substr(0, maxSize);
        }
        else if (stringSize > maxSize + (currentLine - 1) * (maxSize - indentation))
        {
            const auto startPosition = maxSize + (currentLine - 1) * (maxSize - indentation);

            stream << indentationString << std::left << std::setw(maxSize - indentation)
                   << data.substr(startPosition, maxSize - indentation);
        }
        else
        {
            stream << std::left << std::setw(maxSize) << "";
        }

        needsLineBreak |= (stringSize > maxSize + (currentLine) * (maxSize - indentation));

        return stream.str();
    };

    for (auto& sender : senderPortData)
    {
        std::string m_sampleSize{std::to_string(sender.throughputData->m_sampleSize)};
        std::string m_chunkSize{std::to_string(sender.throughputData->m_chunkSize)};
        std::string m_chunksPerMinute{std::to_string(sender.throughputData->m_chunksPerMinute)};
        std::string sendInterval{std::to_string(sender.throughputData->m_lastSendIntervalInNanoseconds / 1000000)};

        currentLine = 0;
        do
        {
            needsLineBreak = false;
            wprintw(pad, " %s |", printEntry(serviceWidth, sender.portData->m_caproServiceID).c_str());
            wprintw(pad, " %s |", printEntry(instanceWidth, sender.portData->m_caproInstanceID).c_str());
            wprintw(pad, " %s |", printEntry(eventWidth, sender.portData->m_caproEventMethodID).c_str());
            wprintw(pad, " %s |", printEntry(processNameWidth, sender.portData->m_name).c_str());
            wprintw(pad, " %s |", printEntry(sampleSizeWidth, m_sampleSize).c_str());
            wprintw(pad, " %s |", printEntry(chunkSizeWidth, m_chunkSize).c_str());
            wprintw(pad, " %s |", printEntry(chunksWidth, m_chunksPerMinute).c_str());
            wprintw(pad, " %s |", printEntry(intervalWidth, sendInterval).c_str());
            wprintw(pad, " %s\n", printEntry(isFieldWidth, (sender.throughputData->m_isField ? "X" : "")).c_str());
            currentLine++;
        } while (needsLineBreak);
    }
    wprintw(pad, "\n");

    constexpr int32_t processUsedWidth{-41};
    prettyPrint("Receiver Ports\n", PrettyOptions::bold);

    wprintw(pad, " %*s |", serviceWidth, "Service");
    wprintw(pad, " %*s |", instanceWidth, "Instance");
    wprintw(pad, " %*s |", eventWidth, "Event");
    wprintw(pad, " %*s |", subscriptionStateWidth, "Subscription");
    wprintw(pad, " %*s |", fifoWidth, "FiFo");
    wprintw(pad, " %*s |", callbackActiveWidth, "Callback");
    wprintw(pad, " %*s |", scopeWidth, "Propagation");
    wprintw(pad, " %*s\n", processUsedWidth, "used by process");

    wprintw(pad, " %*s |", serviceWidth, "");
    wprintw(pad, " %*s |", instanceWidth, "");
    wprintw(pad, " %*s |", eventWidth, "");
    wprintw(pad, " %*s |", subscriptionStateWidth, "State");
    wprintw(pad, " %*s |", fifoWidth, "size / capacity");
    wprintw(pad, " %*s |", callbackActiveWidth, "");
    wprintw(pad, " %*s |", scopeWidth, "scope");
    wprintw(pad, " %*s\n", processUsedWidth, "   ^--- connected to sender port process");

    wprintw(pad, "---------------------------------------------------------------------------------------------------");
    wprintw(pad, "---------------------------------------------------\n");

    auto subscriptionStateToString = [](iox::SubscribeState subState) -> std::string {
        switch (subState)
        {
        case iox::SubscribeState::NOT_SUBSCRIBED:
            return "NOT_SUBSCRIBED";
            break;
        case iox::SubscribeState::SUBSCRIBE_REQUESTED:
            return "SUB_REQUEST";
            break;
        case iox::SubscribeState::SUBSCRIBED:
            return "SUBSCRIBED";
            break;
        case iox::SubscribeState::UNSUBSCRIBE_REQUESTED:
            return "UNSUB_REQUEST";
            break;
        case iox::SubscribeState::WAIT_FOR_OFFER:
            return "WAIT_FOR_OFFER";
            break;
        default:
            return "UNKNOWN";
        }
    };

    for (auto& receiver : receiverPortData)
    {
        currentLine = 0;
        do
        {
            needsLineBreak = false;
            wprintw(pad, " %s |", printEntry(serviceWidth, receiver.portData->m_caproServiceID).c_str());
            wprintw(pad, " %s |", printEntry(instanceWidth, receiver.portData->m_caproInstanceID).c_str());
            wprintw(pad, " %s |", printEntry(eventWidth, receiver.portData->m_caproEventMethodID).c_str());
            wprintw(pad,
                    " %s |",
                    printEntry(subscriptionStateWidth,
                               subscriptionStateToString(receiver.receiverPortChangingData->subscriptionState))
                        .c_str());
            if (currentLine == 0)
            {
                wprintw(
                    pad,
                    " %s / %s |",
                    printEntry(((fifoWidth / 2) - 1), std::to_string(receiver.receiverPortChangingData->fifoSize))
                        .c_str(),
                    printEntry(((fifoWidth / 2) - 1), std::to_string(receiver.receiverPortChangingData->fifoCapacity))
                        .c_str());
            }
            else
            {
                wprintw(pad, " %*s |", fifoWidth, "");
            }
            wprintw(pad,
                    " %s |",
                    printEntry(callbackActiveWidth,
                               (receiver.receiverPortChangingData->sampleSendCallbackActive) ? "X" : "")
                        .c_str());
            wprintw(pad,
                    " %s |",
                    printEntry(scopeWidth,
                               std::string(capro::ScopeTypeString[static_cast<std::underlying_type<capro::Scope>::type>(
                                   receiver.receiverPortChangingData->propagationScope)]))
                        .c_str());

            wprintw(pad, " %s\n", printEntry(processUsedWidth, receiver.portData->m_name).c_str());
            currentLine++;
        } while (needsLineBreak);

        wprintw(pad, " %*s |", serviceWidth, "");
        wprintw(pad, " %*s |", instanceWidth, "");
        wprintw(pad, " %*s |", eventWidth, "");
        wprintw(pad, " %*s |", subscriptionStateWidth, "");
        wprintw(pad, " %*s |", fifoWidth, "");
        wprintw(pad, " %*s |", callbackActiveWidth, "");
        wprintw(pad, " %*s |", scopeWidth, "");
        wprintw(pad, "    ^--- ");

        if (receiver.correspondingSenderPort != nullptr)
        {
            // use the not sorted portData, because the m_senderIndex refers to the original unsorted data
            prettyPrint(receiver.correspondingSenderPort->m_name);
        }
        else
        {
            prettyPrint("disconnected", PrettyOptions::error);
        }
        wprintw(pad, "\n");
    }
}

bool IntrospectionApp::waitForSubscription(SubscriberType& port)
{
    int numberOfLoopsTillTimeout{100};
    bool subscribed{false};
    while ((subscribed = (port.getSubscriptionState() == iox::popo::SubscriptionState::SUBSCRIBED)),
           !subscribed && numberOfLoopsTillTimeout > 0)
    {
        numberOfLoopsTillTimeout--;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return subscribed;
}

std::vector<ComposedSenderPortData>
IntrospectionApp::composeSenderPortData(const PortIntrospectionFieldTopic* portData,
                                        const PortThroughputIntrospectionFieldTopic* throughputData)
{
    std::vector<ComposedSenderPortData> senderPortData;
    senderPortData.reserve(portData->m_senderList.size());

    const PortThroughputData dummyThroughputData;

    auto& m_senderList = portData->m_senderList;
    auto& m_throughputList = throughputData->m_throughputList;
    const bool fastLookup = (m_senderList.size() == m_throughputList.size());
    for (uint64_t i = 0; i < m_senderList.size(); ++i)
    {
        bool found = (fastLookup && m_senderList[i].m_senderPortID == m_throughputList[i].m_senderPortID);
        if (found)
        {
            senderPortData.push_back({m_senderList[i], m_throughputList[i]});
            continue;
        }
        else
        {
            for (const auto& throughput : m_throughputList)
            {
                if (m_senderList[i].m_senderPortID == throughput.m_senderPortID)
                {
                    senderPortData.push_back({m_senderList[i], throughput});
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                senderPortData.push_back({m_senderList[i], dummyThroughputData});
            }
        }
    }

    auto senderSortCriterion = [](const ComposedSenderPortData& sender1, const ComposedSenderPortData& sender2) {
        std::string name1(sender1.portData->m_name.to_cstring());
        std::string name2(sender2.portData->m_name.to_cstring());
        return name1.compare(name2) < 0;
    };
    std::sort(senderPortData.begin(), senderPortData.end(), senderSortCriterion);

    return senderPortData;
}

std::vector<ComposedReceiverPortData>
IntrospectionApp::composeReceiverPortData(const PortIntrospectionFieldTopic* portData,
                                          const ReceiverPortChangingIntrospectionFieldTopic* receiverPortChangingData)
{
    std::vector<ComposedReceiverPortData> receiverPortData;
    receiverPortData.reserve(portData->m_receiverList.size());

    int i = 0;
    if (portData->m_receiverList.size() == receiverPortChangingData->receiverPortChangingDataList.size())
    { // should be the same, else it will be soon
        for (const auto& port : portData->m_receiverList)
        {
            receiverPortData.push_back(
                {port,
                 (port.m_senderIndex != -1) ? &portData->m_senderList[port.m_senderIndex] : nullptr,
                 receiverPortChangingData->receiverPortChangingDataList[i++]});
        }
    }

    auto receiverSortCriterion = [](const ComposedReceiverPortData& receiver1,
                                    const ComposedReceiverPortData& receiver2) {
        std::string name1(receiver1.portData->m_name.to_cstring());
        std::string name2(receiver2.portData->m_name.to_cstring());
        return name1.compare(name2) < 0;
    };
    std::sort(receiverPortData.begin(), receiverPortData.end(), receiverSortCriterion);

    return receiverPortData;
}

void IntrospectionApp::runIntrospection(const iox::units::Duration updatePeriodMs,
                                        const IntrospectionSelection introspectionSelection)
{
    iox::runtime::PoshRuntime::getInstance(iox::roudi::INTROSPECTION_MQ_APP_NAME);

    using namespace iox::roudi;

    initTerminal();
    prettyPrint("### Iceoryx Introspection Client ###\n\n", PrettyOptions::title);

    // mempool
    SubscriberType memPoolSubscriber(IntrospectionMempoolService);
    if (introspectionSelection.mempool == true)
    {
        memPoolSubscriber.subscribe(iox::MAX_SHM_SEGMENTS + 1);

        if (waitForSubscription(memPoolSubscriber) == false)
        {
            prettyPrint("Timeout while waiting for subscription for mempool introspection data!\n",
                        PrettyOptions::error);
        }
    }


    // process
    SubscriberType processSubscriber(IntrospectionProcessService);
    if (introspectionSelection.process == true)
    {
        processSubscriber.subscribe(1);

        if (waitForSubscription(processSubscriber) == false)
        {
            prettyPrint("Timeout while waiting for subscription for process introspection data!\n",
                        PrettyOptions::error);
        }
    }

    // port
    SubscriberType portSubscriber(IntrospectionPortService);

    SubscriberType portThroughputSubscriber(IntrospectionPortThroughputService);

    SubscriberType receiverPortChangingDataSubscriber(IntrospectionReceiverPortChangingDataService);

    if (introspectionSelection.port == true)
    {
        portSubscriber.subscribe(1);
        portThroughputSubscriber.subscribe(1);
        receiverPortChangingDataSubscriber.subscribe(1);

        if (waitForSubscription(portSubscriber) == false)
        {
            prettyPrint("Timeout while waiting for subscription for port introspection data!\n", PrettyOptions::error);
        }
        if (waitForSubscription(portThroughputSubscriber) == false)
        {
            prettyPrint("Timeout while waiting for subscription for port throughput introspection data!\n",
                        PrettyOptions::error);
        }
        if (waitForSubscription(receiverPortChangingDataSubscriber) == false)
        {
            prettyPrint("Timeout while waiting for Subscription for Receiver Port Introspection Changing Data!\n",
                        PrettyOptions::error);
        }
    }

    // Refresh once in case of timeout messages
    refreshTerminal();

    const void* rawProcessSample{nullptr};
    const ProcessIntrospectionFieldTopic* typedProcessSample{nullptr};

    const void* rawPortSample{nullptr};
    const PortIntrospectionFieldTopic* typedPortSample{nullptr};

    const void* rawPortThroughputSample{nullptr};
    const PortThroughputIntrospectionFieldTopic* typedPortThroughputSample{nullptr};

    const void* rawReceiverPortChangingDataSamples{nullptr};
    const ReceiverPortChangingIntrospectionFieldTopic* typedReceiverPortChangingDataSamples{nullptr};

    while (true)
    {
        // get and print time
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        char timeBuf[128];
        strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %X", std::localtime(&in_time_t));
        prettyPrint("### Iceoryx Introspection Client ### ", PrettyOptions::title);
        prettyPrint(timeBuf, PrettyOptions::bold);
        prettyPrint("\n\n", PrettyOptions::bold);

        // print mempool information
        if (introspectionSelection.mempool == true)
        {
            prettyPrint("### MemPool Status ###\n\n", PrettyOptions::highlight);

            std::deque<const MemPoolIntrospectionTopic*> mempoolSamples;
            const void* rawMempoolSample{nullptr};


            while (memPoolSubscriber.getChunk(&rawMempoolSample))
            {
                decltype(mempoolSamples)::value_type typedMempoolSample =
                    static_cast<decltype(mempoolSamples)::value_type>(rawMempoolSample);
                mempoolSamples.push_back(std::move(typedMempoolSample));
                memPoolSubscriber.releaseChunk(rawMempoolSample);
            }

            if (mempoolSamples.empty())
            {
                prettyPrint("Waiting for mempool introspection data ...\n");
            }
            else
            {
                auto it = mempoolSamples.end() - 1;
                auto currentIter = it;
                auto itBegin = mempoolSamples.begin();

                uint32_t lastId = (*it)->m_id;

                while (it != itBegin)
                {
                    --it;
                    if ((*it)->m_id == lastId)
                    {
                        break;
                    }
                    currentIter = it;
                }

                std::for_each(
                    currentIter, mempoolSamples.end(), [&](decltype(*it) samplePtr) { printMemPoolInfo(*samplePtr); });
            }
        }

        // print process information
        if (introspectionSelection.process == true)
        {
            prettyPrint("### Processes ###\n\n", PrettyOptions::highlight);
            if (!processSubscriber.hasNewChunks())
            {
                // No new data sent, hence print the old data
                if (typedProcessSample != nullptr)
                {
                    printProcessIntrospectionData(typedProcessSample);
                }
                else
                {
                    prettyPrint("Waiting for process introspection data ...\n");
                }
            }
            else
            {
                if (processSubscriber.getChunk(&rawProcessSample))
                {
                    typedProcessSample = static_cast<const ProcessIntrospectionFieldTopic*>(rawProcessSample);
                    printProcessIntrospectionData(typedProcessSample);
                    processSubscriber.releaseChunk(rawProcessSample);
                }
            }
        }

        // print port information
        if (introspectionSelection.port == true)
        {
            bool newPortSampleArrived{false};
            bool newPortThroughputSampleeArrived{false};
            bool newReceiverPortChangingDataSamplesArrived{false};

            if (portSubscriber.getChunk(&rawPortSample))
            {
                typedPortSample = static_cast<const PortIntrospectionFieldTopic*>(rawPortSample);
                newPortSampleArrived = true;
            }
            if (portThroughputSubscriber.getChunk(&rawPortThroughputSample))
            {
                typedPortThroughputSample =
                    static_cast<const PortThroughputIntrospectionFieldTopic*>(rawPortThroughputSample);
                newPortThroughputSampleeArrived = true;
            }
            if (receiverPortChangingDataSubscriber.getChunk(&rawReceiverPortChangingDataSamples))
            {
                typedReceiverPortChangingDataSamples =
                    static_cast<const ReceiverPortChangingIntrospectionFieldTopic*>(rawReceiverPortChangingDataSamples);
                newReceiverPortChangingDataSamplesArrived = true;
            }

            if (typedPortSample != nullptr && typedPortThroughputSample != nullptr
                && typedReceiverPortChangingDataSamples != nullptr)
            {
                prettyPrint("### Connections ###\n\n", PrettyOptions::highlight);

                auto composedSenderPortData = composeSenderPortData(typedPortSample, typedPortThroughputSample);
                auto composedReceiverPortData =
                    composeReceiverPortData(typedPortSample, typedReceiverPortChangingDataSamples);

                printPortIntrospectionData(composedSenderPortData, composedReceiverPortData);
            }
            else
            {
                prettyPrint("Waiting for port introspection data ...\n");
            }

            if (newPortSampleArrived)
            {
                portSubscriber.releaseChunk(rawPortSample);
            }
            if (newPortThroughputSampleeArrived)
            {
                portThroughputSubscriber.releaseChunk(rawPortThroughputSample);
            }
            if (newReceiverPortChangingDataSamplesArrived)
            {
                receiverPortChangingDataSubscriber.releaseChunk(rawReceiverPortChangingDataSamples);
            }
        }

        prettyPrint("\n");
        clearToBottom();
        refreshTerminal();

        // Watch user input for updatePeriodMs
        auto tWaitRemaining = std::chrono::milliseconds(updatePeriodMs.milliSeconds<uint64_t>());
        auto tWaitBegin = std::chrono::system_clock::now();
        while (tWaitRemaining.count() >= 0)
        {
            waitForUserInput(static_cast<int32_t>(tWaitRemaining.count()));
            auto tWaitElapsed = std::chrono::system_clock::now() - tWaitBegin;
            tWaitRemaining = std::chrono::milliseconds(updatePeriodMs.milliSeconds<uint64_t>())
                             - std::chrono::duration_cast<std::chrono::milliseconds>(tWaitElapsed);
        }
    }

    getchar();
    closeTerminal();
}

} // namespace introspection
} // namespace client
} // namespace iox
