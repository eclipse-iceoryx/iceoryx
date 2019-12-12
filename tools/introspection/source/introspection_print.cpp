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

#include "iceoryx_introspection/introspection_print.hpp"
#include <iomanip>
#include <poll.h>

namespace iox
{
namespace client
{
namespace introspection
{
/// @brief ncurses pad
static WINDOW* pad;
/// @brief first pad row to show on the ncurses window
static int32_t yPad = 0;
/// @brief first pad column to show on the ncurses window
static int32_t xPad = 0;

void initTerminal()
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

void closeTerminal()
{
    endwin();
}

void refreshTerminal()
{
    prefresh(pad, yPad, xPad, 0, 0, LINES - 1, COLS - 1);

    constexpr int32_t titleLines{0};
    wmove(pad, titleLines, 0);
}

/// @brief updates the first pad coordinates to display
void updateDisplayYX()
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

void waitForUserInput(int32_t timeoutMs)
{
    struct pollfd fileDesc;
    fileDesc.fd = STDIN_FILENO;
    fileDesc.events = POLLIN;
    constexpr size_t nFileDesc = 1;
    int32_t eventCount = poll(&fileDesc, nFileDesc, timeoutMs);

    // Event detected
    if ((eventCount == nFileDesc) && (fileDesc.revents == POLLIN))
    {
        updateDisplayYX();
        refreshTerminal();
    }
}

void prettyPrint(const std::string& str, const PrettyOptions pr)
{
    wattron(pad, prettyMap.find(pr)->second);
    wprintw(pad, str.c_str());
    wattroff(pad, prettyMap.find(pr)->second);
}

void printProcessIntrospectionData(const ProcessIntrospectionFieldTopic& processIntrospectionField)
{
    constexpr int32_t pidWidth{-10};
    constexpr int32_t processWidth{-10};

    for (auto& data : processIntrospectionField.m_processList)
    {
        wprintw(pad, "PID: %*d Process: %*s\n", pidWidth, data.m_pid, processWidth, data.m_name);
    }
    wprintw(pad, "\n");
}

void printMemPoolInfo(const MemPoolIntrospectionTopic& topic)
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

void printPortIntrospectionData(const std::vector<ComposedSenderPortData>& senderPortData,
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

} // namespace introspection
} // namespace client
} // namespace iox
