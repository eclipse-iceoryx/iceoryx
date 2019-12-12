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
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

namespace iox
{
namespace runtime
{
SharedMemoryUser::SharedMemoryUser(std::string baseAddrString[[gnu::unused]],
                                   const bool doMapSharedMemoryIntoThread,
                                   const size_t topicSize,
                                   std::string segmentManagerAddr,
                                   const uint64_t segmentId)
{
    if (doMapSharedMemoryIntoThread)
    {
        // we let the OS decide where to map the shm segments
        constexpr void* BASE_ADDRESS_HINT{nullptr};

        // create and map the already existing shared memory region
        m_shmObject = posix::SharedMemoryObject::create(
            SHM_NAME, topicSize, posix::AccessMode::readWrite, posix::OwnerShip::openExisting, BASE_ADDRESS_HINT);

        if (!m_shmObject.has_value())
        {
            errorHandler(Error::kPOSH__SHM_APP_MAPP_ERR);
        }

        RelativePointer::registerPtr(segmentId, m_shmObject->getBaseAddress(), m_shmObject->getSizeInBytes());

        LogInfo() << "Application registered management segment "
                  << iox::log::HexFormat(reinterpret_cast<uint64_t>(m_shmObject->getBaseAddress())) << " with size "
                  << m_shmObject->getSizeInBytes() << " to id " << segmentId;

        auto ptr = RelativePointer::getPtr(segmentId, std::stoll(segmentManagerAddr));
        auto segmentManager = reinterpret_cast<mepoo::SegmentManager<>*>(ptr);

        auto segmentMapping = segmentManager->getSegmentMappings(posix::PosixUser::getUserOfCurrentProcess());
        for (const auto& segment : segmentMapping)
        {
            auto accessMode = segment.m_isWritable ? posix::AccessMode::readWrite : posix::AccessMode::readOnly;
            auto shmObject = posix::SharedMemoryObject::create(segment.m_sharedMemoryName.c_str(),
                                                               segment.m_size,
                                                               accessMode,
                                                               posix::OwnerShip::openExisting,
                                                               BASE_ADDRESS_HINT);
            if (shmObject.has_value())
            {
                if (static_cast<uint32_t>(m_payloadShmObjects.size()) >= MAX_SHM_SEGMENTS)
                {
                    errorHandler(Error::kPOSH__SHM_APP_SEGMENT_COUNT_OVERFLOW);
                }

                RelativePointer::registerPtr(
                    segment.m_segmentId, shmObject->getBaseAddress(), shmObject->getSizeInBytes());


                LogInfo() << "Application registered payload segment "
                          << iox::log::HexFormat(reinterpret_cast<uint64_t>(shmObject->getBaseAddress()))
                          << " with size " << shmObject->getSizeInBytes() << " to id " << segment.m_segmentId;

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
