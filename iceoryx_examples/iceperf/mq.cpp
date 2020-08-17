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

#include "mq.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/internal/posix_wrapper/message_queue.hpp"
#include "iceoryx_utils/platform/fcntl.hpp"
#include "iceoryx_utils/platform/platform_correction.hpp"

#include <chrono>

MQ::MQ(const std::string& publisherName, const std::string& subscriberName) noexcept
    : m_publisherName(publisherName)
    , m_subscriberName(subscriberName)
{
}

void MQ::initLeader() noexcept
{
    init();

    open(m_publisherName, iox::posix::IpcChannelSide::SERVER);

    std::cout << "waiting for follower" << std::endl;

    // receivePerfTopic();
}

void MQ::initFollower() noexcept
{
    init();

    open(m_publisherName, iox::posix::IpcChannelSide::SERVER);

    std::cout << "registering with the leader, if no leader this will crash with a socket error now" << std::endl;

    // sendPerfTopic(sizeof(PerfTopic), true);
}

void MQ::shutdown() noexcept
{
}


void MQ::prePingPongLeader(uint32_t payloadSizeInBytes) noexcept
{
}

void MQ::postPingPongLeader() noexcept
{
}

void MQ::triggerEnd() noexcept
{
}

double MQ::pingPongLeader(int64_t numRoundTrips) noexcept
{
    return 0.0;
}

void MQ::pingPongFollower() noexcept
{
}

void MQ::init() noexcept
{
    // fields have a different order in QNX,
    // so we need to initialize by name
    m_attributes.mq_flags = 0;
    m_attributes.mq_maxmsg = MAX_MESSAGES;
    m_attributes.mq_msgsize = MAX_MESSAGE_SIZE;
    m_attributes.mq_curmsgs = 0;
#ifdef __QNX__
    m_attributes.mq_recvwait = 0;
    m_attributes.mq_sendwait = 0;
#endif
}

void MQ::open(const std::string& name, const iox::posix::IpcChannelSide channelSide) noexcept
{
    int32_t openFlags = O_RDWR;
    if (channelSide == iox::posix::IpcChannelSide::SERVER)
    {
        auto mqCall = iox::cxx::makeSmartC(
            mq_unlink, iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {ENOENT}, name.c_str());
        if (mqCall.hasErrors())
        {
            std::cout << "mq_unlink error" << std::endl;
            exit(1);
        }

        openFlags |= O_CREAT;
    }

    // the mask will be applied to the permissions, therefore we need to set it to 0
    mode_t umaskSaved = umask(0);

    auto mqCall = iox::cxx::makeSmartC(mq_open,
                                       iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                       {ERROR_CODE},
                                       {ENOENT},
                                       name.c_str(),
                                       openFlags,
                                       m_filemode,
                                       &m_attributes);

    umask(umaskSaved);

    if (mqCall.hasErrors())
    {
        std::cout << "mq_open error " << name << std::endl;
        exit(1);
    }

    if (channelSide == iox::posix::IpcChannelSide::SERVER)
    {
        m_mqDescriptorPublisher = mqCall.getReturnValue();
    }
    else
    {
        m_mqDescriptorSubscriber = mqCall.getReturnValue();
    }
}