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
#ifndef IOX_EXAMPLES_ICEPERF_MQ_HPP
#define IOX_EXAMPLES_ICEPERF_MQ_HPP

#include "base.hpp"

#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/design_pattern/creation.hpp"
#include "iceoryx_utils/fixed_string/string100.hpp"
#include "iceoryx_utils/internal/posix_wrapper/ipc_channel.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/platform/fcntl.hpp"
#include "iceoryx_utils/platform/mqueue.hpp"
#include "iceoryx_utils/platform/stat.hpp"

#include <string>

class MQ : public IcePerfBase
{
  public:
    static constexpr size_t MAX_MESSAGE_SIZE = 4 * IcePerfBase::ONE_KILOBYTE;
    static constexpr size_t MAX_MESSAGES = 8;
    static constexpr int32_t ERROR_CODE = -1;
    static constexpr mqd_t INVALID_DESCRIPTOR = -1;

    MQ(const std::string& publisherName, const std::string& subscriberName) noexcept;
    void initLeader() noexcept override;
    void initFollower() noexcept override;
    void shutdown() noexcept override;

  private:
    void init() noexcept;
    void open(const std::string& name, const iox::posix::IpcChannelSide channelSide) noexcept;
    void send(const char* buffer, uint32_t length) noexcept;
    void receive(char* buffer) noexcept;
    void sendPerfTopic(uint32_t payloadSizeInBytes, bool runFlag) noexcept override;
    PerfTopic receivePerfTopic() noexcept override;

    const std::string m_publisherName;
    const std::string m_subscriberName;
    struct mq_attr m_attributes;
    mqd_t m_mqDescriptorPublisher = INVALID_DESCRIPTOR;
    mqd_t m_mqDescriptorSubscriber = INVALID_DESCRIPTOR;
    // read/write permissions
    static constexpr mode_t m_filemode{S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH};
    char m_message[MAX_MESSAGE_SIZE];
};

#endif // IOX_EXAMPLES_ICEPERF_MQ_HPP
