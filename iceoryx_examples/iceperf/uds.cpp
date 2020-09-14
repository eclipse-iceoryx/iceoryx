// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "uds.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"

#include <chrono>

UDS::UDS(const std::string& publisherName, const std::string& subscriberName) noexcept
    : m_publisherName(publisherName)
    , m_subscriberName(subscriberName)
{
}

void UDS::initLeader() noexcept
{
    init();

    std::cout << "waiting for follower" << std::endl;

    receivePerfTopic();
}

void UDS::initFollower() noexcept
{
    init();

    std::cout << "registering with the leader, if no leader this will crash with a socket error now" << std::endl;

    sendPerfTopic(sizeof(PerfTopic), true);
}

void UDS::init() noexcept
{
    // initialize the sockAddr data structure with the provided name
    memset(&m_sockAddrPublisher, 0, sizeof(m_sockAddrPublisher));
    m_sockAddrPublisher.sun_family = AF_LOCAL;
    strncpy(m_sockAddrPublisher.sun_path, m_publisherName.c_str(), m_publisherName.size());

    auto socketCallPublisher = iox::cxx::makeSmartC(
        socket, iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {}, AF_LOCAL, SOCK_DGRAM, 0);

    if (socketCallPublisher.hasErrors())
    {
        std::cout << "socket error" << std::endl;
        exit(1);
    }

    m_sockfdPublisher = socketCallPublisher.getReturnValue();

    // initialize the sockAddr data structure with the provided name
    memset(&m_sockAddrSubscriber, 0, sizeof(m_sockAddrSubscriber));
    m_sockAddrSubscriber.sun_family = AF_LOCAL;
    strncpy(m_sockAddrSubscriber.sun_path, m_subscriberName.c_str(), m_subscriberName.size());

    auto socketCallSubscriber = iox::cxx::makeSmartC(
        socket, iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {}, AF_LOCAL, SOCK_DGRAM, 0);

    if (socketCallSubscriber.hasErrors())
    {
        std::cout << "socket error" << std::endl;
        exit(1);
    }

    m_sockfdSubscriber = socketCallSubscriber.getReturnValue();

    auto unlinkCall = iox::cxx::makeSmartC(
        unlink, iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {ENOENT}, m_sockAddrSubscriber.sun_path);

    if (unlinkCall.hasErrors())
    {
        std::cout << "unlink error" << std::endl;
        exit(1);
    }

    auto bindCall = iox::cxx::makeSmartC(bind,
                                         iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                         {ERROR_CODE},
                                         {},
                                         m_sockfdSubscriber,
                                         reinterpret_cast<struct sockaddr*>(&m_sockAddrSubscriber),
                                         static_cast<socklen_t>(sizeof(m_sockAddrSubscriber)));

    if (bindCall.hasErrors())
    {
        std::cout << "bind error" << std::endl;
        exit(1);
    }
}

void UDS::shutdown() noexcept
{
    if (m_sockfdPublisher != INVALID_FD)
    {
        auto closeCall = iox::cxx::makeSmartC(
            closePlatformFileHandle, iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {}, m_sockfdPublisher);

        if (closeCall.hasErrors())
        {
            std::cout << "close error" << std::endl;
            exit(1);
        }
    }

    if (m_sockfdSubscriber != INVALID_FD)
    {
        auto closeCall = iox::cxx::makeSmartC(closePlatformFileHandle,
                                              iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                              {ERROR_CODE},
                                              {},
                                              m_sockfdSubscriber);

        if (closeCall.hasErrors())
        {
            std::cout << "close error" << std::endl;
            exit(1);
        }

        auto unlinkCall = iox::cxx::makeSmartC(
            unlink, iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {}, m_sockAddrSubscriber.sun_path);

        if (unlinkCall.hasErrors())
        {
            std::cout << "unlink error" << std::endl;
            exit(1);
        }
    }
}

void UDS::sendPerfTopic(uint32_t payloadSizeInBytes, bool runFlag) noexcept
{
    char* buffer = new char[payloadSizeInBytes];
    auto sample = reinterpret_cast<PerfTopic*>(&buffer[0]);

    // Specify the payload size for the measurement
    sample->payloadSize = payloadSizeInBytes;
    sample->run = runFlag;
    if (payloadSizeInBytes <= MAX_MESSAGE_SIZE)
    {
        sample->subPackets = 1;
        send(&buffer[0], payloadSizeInBytes);
    }
    else
    {
        sample->subPackets = payloadSizeInBytes / MAX_MESSAGE_SIZE;
        for (uint32_t i = 0U; i < sample->subPackets; ++i)
        {
            send(&buffer[0], MAX_MESSAGE_SIZE);
        }
    }
    delete[] buffer;
}

PerfTopic UDS::receivePerfTopic() noexcept
{
    receive(&m_message[0]);

    auto receivedSample = reinterpret_cast<const PerfTopic*>(&m_message[0]);

    if (receivedSample->subPackets > 1)
    {
        for (uint32_t i = 0U; i < receivedSample->subPackets - 1; ++i)
        {
            receive(&m_message[0]);
        }
    }

    return *receivedSample;
}

void UDS::send(const char* buffer, uint32_t length) noexcept
{
    auto sendCall = iox::cxx::makeSmartC(sendto,
                                         iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                         {ERROR_CODE},
                                         {},
                                         m_sockfdPublisher,
                                         buffer,
                                         length,
                                         static_cast<int>(0),
                                         reinterpret_cast<struct sockaddr*>(&m_sockAddrPublisher),
                                         static_cast<socklen_t>(sizeof(m_sockAddrPublisher)));

    if (sendCall.hasErrors())
    {
        std::cout << std::endl << "send error" << std::endl;
        exit(1);
    }
}

void UDS::receive(char* buffer) noexcept
{
    auto recvCall = iox::cxx::makeSmartC(recvfrom,
                                         iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                         {ERROR_CODE},
                                         {},
                                         m_sockfdSubscriber,
                                         buffer,
                                         MAX_MESSAGE_SIZE,
                                         0,
                                         nullptr,
                                         nullptr);

    if (recvCall.hasErrors())
    {
        std::cout << "receive error" << std::endl;
        exit(1);
    }
}
