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

#ifndef IOX_POSH_POPO_CLIENT_HPP
#define IOX_POSH_POPO_CLIENT_HPP

#include "iceoryx_posh/internal/popo/client_impl.hpp"

namespace iox
{
namespace popo
{
/// @brief The Client class for the request-response messaging pattern in iceoryx.
/// @param[in] Req type of request data
/// @param[in] Res type of response data
template <typename Req, typename Res>
class Client : public ClientImpl<Req, Res>
{
    using Impl = ClientImpl<Req, Res>;

  public:
    using ClientImpl<Req, Res>::ClientImpl;

    virtual ~Client() noexcept
    {
        Impl::m_trigger.reset();
    }
};
} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_CLIENT_HPP
