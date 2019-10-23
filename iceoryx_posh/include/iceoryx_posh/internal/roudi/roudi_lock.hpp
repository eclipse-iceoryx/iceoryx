// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"

#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace iox
{
namespace roudi
{
class RouDiLock
{
  public:
    RouDiLock();

    ~RouDiLock();

  private:
    int m_socket_fd =
        cxx::makeSmartC(socket, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, AF_INET, SOCK_STREAM, 0)
            .getReturnValue();
    struct sockaddr_in m_sockserv;
};

} // namespace roudi
} // namespace iox
