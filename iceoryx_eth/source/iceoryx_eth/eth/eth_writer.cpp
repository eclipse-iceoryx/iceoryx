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

#include "eth/eth_writer.hpp"
#include "eth/eth_context.hpp"
#include "internal/log/logging.hpp"

//#include <Mempool_DCPS.hpp>
#include <string>
extern "C"{
    #include "eth/ipc-shm.h"
}
iox::eth::ethDataWriter::ethDataWriter(IdString serviceId, IdString instanceId, IdString eventId)
    : m_serviceId(serviceId)
    , m_instanceId(instanceId)
    , m_eventId(eventId)
{
    LogDebug() << "[ethDataWriter] Created ethDataWriter.";
}

iox::eth::ethDataWriter::~ethDataWriter()
{

}

void iox::eth::ethDataWriter::connect() noexcept
{
   std::cout << "Testing the connect" << std::endl ;  
}

void iox::eth::ethDataWriter::write(const uint8_t* const bytes, const uint64_t size) noexcept
{

 std::cout << "Testing the write" << std::endl ;   
 // ipc_shm_tx(1, 0, 10);

}
iox::eth::IdString iox::eth::ethDataWriter::getServiceId() const noexcept
{
    return m_serviceId;
}

iox::eth::IdString iox::eth::ethDataWriter::getInstanceId() const noexcept
{
    return m_instanceId;
}

iox::eth::IdString iox::eth::ethDataWriter::getEventId() const noexcept
{
    return m_eventId;
}

