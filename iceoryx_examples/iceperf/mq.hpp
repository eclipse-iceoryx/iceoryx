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

    MQ(const std::string& publisherName, const std::string& subscriberName) noexcept;
    void initLeader() noexcept override;
    void initFollower() noexcept override;
    void shutdown() noexcept override;
    void prePingPongLeader(uint32_t payloadSizeInBytes) noexcept override;
    void postPingPongLeader() noexcept override;
    void triggerEnd() noexcept override;
    double pingPongLeader(int64_t numRoundTrips) noexcept override;
    void pingPongFollower() noexcept override;

  private:

};

#endif // IOX_EXAMPLES_ICEPERF_UDS_HPP