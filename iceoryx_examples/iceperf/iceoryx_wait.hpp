// Copyright 2023, Eclipse Foundation and the iceoryx contributors. All rights reserved.
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
#ifndef IOX_EXAMPLES_ICEPERF_ICEORYX_WAIT_HPP
#define IOX_EXAMPLES_ICEPERF_ICEORYX_WAIT_HPP

#include "iceoryx.hpp"

class IceoryxWait : public Iceoryx
{
  public:
    IceoryxWait(const iox::capro::IdString_t& publisherName, const iox::capro::IdString_t& subscriberName) noexcept;

  private:
    void init() noexcept override;
    PerfTopic receivePerfTopic() noexcept override;

    iox::popo::WaitSet<> waitset;
};

#endif // IOX_EXAMPLES_ICEPERF_ICEORYX_WAIT_HPP
