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
#ifndef IOX_EXAMPLES_ICEPERF_ICEORYX_HPP
#define IOX_EXAMPLES_ICEPERF_ICEORYX_HPP

#include "base.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"

class Iceoryx : public IcePerfBase
{
  public:
    Iceoryx(const iox::capro::IdString& publisherName, const iox::capro::IdString& subscriberName) noexcept;
    void initLeader() noexcept override;
    void initFollower() noexcept override;
    void shutdown() noexcept override;

  private:
    void init() noexcept;
    void sendPerfTopic(uint32_t payloadSizeInBytes, bool runFlag) noexcept override;
    PerfTopic receivePerfTopic() noexcept override;

    iox::popo::Publisher m_publisher;
    iox::popo::Subscriber m_subscriber;
};

#endif // IOX_EXAMPLES_ICEPERF_ICEORYX_HPP
