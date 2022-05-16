// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_METHOD_CLIENT_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_METHOD_CLIENT_HPP

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_posh/popo/client.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"

#include "owl/types.hpp"
#include "topic.hpp"

namespace owl
{
namespace kom
{
class MethodClient
{
  public:
    MethodClient(const core::String& service, const core::String& instance, const core::String& event) noexcept;
    ~MethodClient() noexcept;

    MethodClient(const MethodClient&) = delete;
    MethodClient(MethodClient&&) = delete;
    MethodClient& operator=(const MethodClient&) = delete;
    MethodClient& operator=(MethodClient&&) = delete;

    Future<AddResponse> operator()(uint64_t addend1, uint64_t addend2);

  private:
    //! [MethodClient members]
    iox::popo::Client<AddRequest, AddResponse> m_client;
    std::atomic<int64_t> m_sequenceId{0};
    iox::popo::WaitSet<> m_waitset;
    static constexpr bool isRecursive{true};
    iox::posix::mutex m_mutex{isRecursive};
    //! [MethodClient members]
};
} // namespace kom
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_METHOD_CLIENT_HPP
