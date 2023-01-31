// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/runtime/ipc_runtime_interface.hpp"
#include "iceoryx_dust/cxx/convert.hpp"
#include "iceoryx_dust/cxx/std_string_support.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_posh/error_handling/error_handling.hpp"
#include "iceoryx_posh/version/version_info.hpp"
#include "iox/into.hpp"

#include <thread>

namespace iox
{
namespace runtime
{
IpcRuntimeInterface::IpcRuntimeInterface(const RuntimeName_t& roudiName,
                                         const RuntimeName_t& runtimeName,
                                         const units::Duration roudiWaitingTimeout) noexcept
    : m_runtimeName(runtimeName)
    , m_RoudiIpcInterface(roudiName)
{
    m_AppIpcInterface.emplace(runtimeName);
    if (!m_AppIpcInterface->isInitialized())
    {
        errorHandler(PoshError::IPC_INTERFACE__UNABLE_TO_CREATE_APPLICATION_CHANNEL);
        return;
    }

    deadline_timer timer(roudiWaitingTimeout);

    enum class RegState
    {
        WAIT_FOR_ROUDI,
        SEND_REGISTER_REQUEST,
        WAIT_FOR_REGISTER_ACK,
        FINISHED
    };

    int64_t transmissionTimestamp{0};
    auto regState = RegState::WAIT_FOR_ROUDI;
    while (!timer.hasExpired() && regState != RegState::FINISHED)
    {
        if (!m_RoudiIpcInterface.isInitialized() || !m_RoudiIpcInterface.ipcChannelMapsToFile())
        {
            LogDebug() << "reopen RouDi's IPC channel!";
            m_RoudiIpcInterface.reopen();
            regState = RegState::WAIT_FOR_ROUDI;
        }

        switch (regState)
        {
        case RegState::WAIT_FOR_ROUDI:
        {
            waitForRoudi(timer);
            if (m_RoudiIpcInterface.isInitialized())
            {
                regState = RegState::SEND_REGISTER_REQUEST;
            }
            break;
        }
        case RegState::SEND_REGISTER_REQUEST:
        {
            using namespace units;
            using namespace std::chrono;
            auto timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
            while (transmissionTimestamp == timestamp)
            {
                timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
            }
            transmissionTimestamp = timestamp;

            // send IpcMessageType::REG to RouDi

            IpcMessage sendBuffer;
            int pid = getpid();
            cxx::Expects(pid >= 0);
            sendBuffer << IpcMessageTypeToString(IpcMessageType::REG) << m_runtimeName << cxx::convert::toString(pid)
                       << cxx::convert::toString(posix::PosixUser::getUserOfCurrentProcess().getID())
                       << cxx::convert::toString(transmissionTimestamp)
                       << static_cast<cxx::Serialization>(version::VersionInfo::getCurrentVersion()).toString();

            bool successfullySent = m_RoudiIpcInterface.timedSend(sendBuffer, 100_ms);

            if (successfullySent)
            {
                regState = RegState::WAIT_FOR_REGISTER_ACK;
            }
            else
            {
                regState = RegState::WAIT_FOR_ROUDI;
            }
            break;
        }
        case RegState::WAIT_FOR_REGISTER_ACK:
            if (waitForRegAck(transmissionTimestamp) == RegAckResult::SUCCESS)
            {
                regState = RegState::FINISHED;
            }
            else
            {
                regState = RegState::WAIT_FOR_ROUDI;
            }
            break;
        case RegState::FINISHED:
            // nothing to do, move along
            break;
        }
    }

    if (regState != RegState::FINISHED)
    {
        m_AppIpcInterface.reset();
    }
    switch (regState)
    {
    case RegState::WAIT_FOR_ROUDI:
        LogFatal() << "Timeout registering at RouDi. Is RouDi running?";
        errorHandler(PoshError::IPC_INTERFACE__REG_ROUDI_NOT_AVAILABLE);
        break;
    case RegState::SEND_REGISTER_REQUEST:
        errorHandler(PoshError::IPC_INTERFACE__REG_UNABLE_TO_WRITE_TO_ROUDI_CHANNEL);
        break;
    case RegState::WAIT_FOR_REGISTER_ACK:
        errorHandler(PoshError::IPC_INTERFACE__REG_ACK_NO_RESPONSE);
        break;
    case RegState::FINISHED:
        // nothing to do, move along
        break;
    }
}

bool IpcRuntimeInterface::sendKeepalive() noexcept
{
    return (m_sendKeepalive) ? m_RoudiIpcInterface.send(
               {IpcMessageTypeToString(IpcMessageType::KEEPALIVE), into<std::string>(m_runtimeName)})
                             : true;
}

memory::UntypedRelativePointer::offset_t IpcRuntimeInterface::getSegmentManagerAddressOffset() const noexcept
{
    cxx::Ensures(m_segmentManagerAddressOffset.has_value()
                 && "No segment manager available! Should have been fetched in the c'tor");
    return m_segmentManagerAddressOffset.value();
}

bool IpcRuntimeInterface::sendRequestToRouDi(const IpcMessage& msg, IpcMessage& answer) noexcept
{
    if (!m_RoudiIpcInterface.send(msg))
    {
        LogError() << "Could not send request via RouDi IPC channel interface.\n";
        return false;
    }

    if (!m_AppIpcInterface->receive(answer))
    {
        LogError() << "Could not receive request via App IPC channel interface.\n";
        return false;
    }

    return true;
}

size_t IpcRuntimeInterface::getShmTopicSize() noexcept
{
    return m_shmTopicSize;
}

void IpcRuntimeInterface::waitForRoudi(deadline_timer& timer) noexcept
{
    bool printWaitingWarning = true;
    bool printFoundMessage = false;
    while (!timer.hasExpired() && !m_RoudiIpcInterface.isInitialized())
    {
        m_RoudiIpcInterface.reopen();

        if (m_RoudiIpcInterface.isInitialized())
        {
            LogDebug() << "RouDi IPC Channel found!";
            break;
        }

        if (printWaitingWarning)
        {
            LogWarn() << "RouDi not found - waiting ...";
            printWaitingWarning = false;
            printFoundMessage = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (printFoundMessage && m_RoudiIpcInterface.isInitialized())
    {
        LogWarn() << "... RouDi found.";
    }
}

IpcRuntimeInterface::RegAckResult IpcRuntimeInterface::waitForRegAck(int64_t transmissionTimestamp) noexcept
{
    // wait for the register ack from the RouDi daemon. If we receive another response we do a retry
    constexpr size_t MAX_RETRY_COUNT = 3;
    size_t retryCounter = 0;
    while (retryCounter++ < MAX_RETRY_COUNT)
    {
        using namespace units::duration_literals;
        IpcMessage receiveBuffer;
        // wait for IpcMessageType::REG_ACK from RouDi for 1 seconds
        if (m_AppIpcInterface->timedReceive(1_s, receiveBuffer))
        {
            std::string cmd = receiveBuffer.getElementAtIndex(0U);

            if (stringToIpcMessageType(cmd.c_str()) == IpcMessageType::REG_ACK)
            {
                constexpr uint32_t REGISTER_ACK_PARAMETERS = 6U;
                if (receiveBuffer.getNumberOfElements() != REGISTER_ACK_PARAMETERS)
                {
                    errorHandler(PoshError::IPC_INTERFACE__REG_ACK_INVALIG_NUMBER_OF_PARAMS);
                }

                // read out the shared memory base address and save it
                iox::cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), m_shmTopicSize);
                memory::UntypedRelativePointer::offset_t offset{0U};
                iox::cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), offset);
                m_segmentManagerAddressOffset.emplace(offset);

                int64_t receivedTimestamp{0U};
                cxx::convert::fromString(receiveBuffer.getElementAtIndex(3U).c_str(), receivedTimestamp);
                cxx::convert::fromString(receiveBuffer.getElementAtIndex(4U).c_str(), m_segmentId);
                cxx::convert::fromString(receiveBuffer.getElementAtIndex(5U).c_str(), m_sendKeepalive);
                if (transmissionTimestamp == receivedTimestamp)
                {
                    return RegAckResult::SUCCESS;
                }
                else
                {
                    LogWarn() << "Received a REG_ACK with an outdated timestamp!";
                }
            }
            else
            {
                LogError() << "Wrong response received " << receiveBuffer.getMessage();
            }
        }
    }

    return RegAckResult::TIMEOUT;
}

uint64_t IpcRuntimeInterface::getSegmentId() const noexcept
{
    return m_segmentId;
}
} // namespace runtime
} // namespace iox
