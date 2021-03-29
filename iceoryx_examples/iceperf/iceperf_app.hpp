// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_EXAMPLES_ICEPERF_HPP
#define IOX_EXAMPLES_ICEPERF_HPP

#include "base.hpp"
#include "example_common.hpp"

#include "iceoryx_posh/iceoryx_posh_types.hpp"

class IcePerfApp {
public:
    IcePerfApp(const PerfSettings settings) noexcept;

    void run() noexcept;

private:
    void run(const iox::capro::IdString_t publisherName, const iox::capro::IdString_t subscriberName) noexcept;
    void doIt(IcePerfBase& ipcTechnology) noexcept;
    void leaderDo(IcePerfBase& ipcTechnology) noexcept;
    void followerDo(IcePerfBase& ipcTechnology) noexcept;

private:
    const PerfSettings m_settings;
};

#endif // IOX_EXAMPLES_ICEPERF_HPP
