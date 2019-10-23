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

#include "iceoryx_posh/internal/runtime/shared_memory_user.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/mepoo/segment_manager.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/internal/posix_wrapper/posix_access_rights.hpp"

namespace iox
{
namespace runtime
{
SharedMemoryUser::SharedMemoryUser(std::string baseAddrString,
                                   const bool doMapSharedMemoryIntoThread,
                                   const size_t topicSize,
                                   std::string segmentManagerAddr)
{
    if (doMapSharedMemoryIntoThread)
    {
        void* baseAddr = reinterpret_cast<void*>(std::stoull(baseAddrString));

        // create and map the already existing shared memory region
        m_shmObject = posix::SharedMemoryObject::create(
            SHM_NAME, topicSize, posix::AccessMode::readWrite, posix::OwnerShip::openExisting, baseAddr);

        if (!m_shmObject.has_value())
        {
            errorHandler(Error::kPOSH__SHM_APP_MAPP_ERR);
        }

        if (m_shmObject->getBaseAddress() != baseAddr)
        {
            LogError()
                << "Application didn't map the shm segment to the same address as RouDi. Currently this is a hard "
                << "error! RouDi has the option -b to set a custom base address hint for your system. Base address "
                << "of RouDi: " << log::HexFormat(reinterpret_cast<uintptr_t>(baseAddr))
                << " Address assigned to the application: "
                << log::HexFormat(reinterpret_cast<uintptr_t>(m_shmObject->getBaseAddress()));
            errorHandler(Error::kPOSH__SHM_APP_BASEADDRESS_VIOLATES_SPECIFICATION);
        }

        auto segmentManager = reinterpret_cast<mepoo::SegmentManager<>*>(std::stoull(segmentManagerAddr));
        auto segmentMapping = segmentManager->getSegmentMappings(posix::PosixUser::getUserOfCurrentProcess());
        for (const auto& segment : segmentMapping)
        {
            auto accessMode = segment.m_isWritable ? posix::AccessMode::readWrite : posix::AccessMode::readOnly;
            auto shmObject = posix::SharedMemoryObject::create(segment.m_sharedMemoryName.c_str(),
                                                               segment.m_size,
                                                               accessMode,
                                                               posix::OwnerShip::openExisting,
                                                               segment.m_startAddress);
            if (shmObject.has_value())
            {
                if (static_cast<uint32_t>(m_payloadShmObjects.size()) >= MAX_SHM_SEGMENTS)
                {
                    errorHandler(Error::kPOSH__SHM_APP_SEGMENT_COUNT_OVERFLOW);
                }
                if (shmObject->getBaseAddress() != segment.m_startAddress)
                {
                    LogError()
                        << "Application didn't map the shm segment to the same address as RouDi. Currently this is "
                        << "a hard error! RouDi has the option -b to set a custom start address hint for your "
                        << "system. Base address of RouDi: " << log::HexFormat(reinterpret_cast<uintptr_t>(baseAddr))
                        << ". Segment address requested by RouDi: "
                        << log::HexFormat(reinterpret_cast<uintptr_t>(segment.m_startAddress))
                        << ". Segment address assigned to the application: "
                        << log::HexFormat(reinterpret_cast<uintptr_t>(shmObject->getBaseAddress()));
                    errorHandler(Error::kPOSH__SHM_APP_SEGMENT_BASEADDRESS_VIOLATES_SPECIFICATION);
                }
                m_payloadShmObjects.emplace_back(std::move(*shmObject));
            }
            else
            {
                errorHandler(Error::kPOSH__SHM_APP_SEGMENT_MAPP_ERR);
            }
        }
    }
}

} // namespace runtime
} // namespace iox
