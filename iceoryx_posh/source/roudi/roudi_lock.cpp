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

#include "iceoryx_posh/internal/roudi/roudi_lock.hpp"

#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace iox
{
namespace roudi
{
RouDiLock::RouDiLock()
{
    if (m_socket_fd == -1)
    {
        LogError()<<"Could not create socket";
        std::terminate();
    }

    m_sockserv.sin_family = AF_INET;
    m_sockserv.sin_addr.s_addr = inet_addr("127.0.0.1");
    m_sockserv.sin_port = htons(37777);

    // bind socket for locking
    auto l_bindcall = cxx::makeSmartC(bind,
                                      cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE,
                                      {0},
                                      {},
                                      m_socket_fd,
                                      reinterpret_cast<struct sockaddr*>(&m_sockserv),
                                      static_cast<unsigned int>(sizeof(m_sockserv)));


    if (l_bindcall.hasErrors())
    {
        LogError()<<"Cannot lock socket, is RouDi already running? \n";
        exit(EXIT_FAILURE);
    }
}

RouDiLock::~RouDiLock()
{
    // Close socket
    auto l_socket_close = cxx::makeSmartC(close, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_socket_fd);

    if (l_socket_close.hasErrors())
    {
        LogError()<< "Could not close socket";
    }
}

} // namespace roudi
} // namespace iox
