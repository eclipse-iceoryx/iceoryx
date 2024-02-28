// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2024 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

#include "iceoryx_posh/internal/runtime/shared_memory_user.hpp"
#include "iceoryx_posh/internal/mepoo/segment_manager.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iox/detail/convert.hpp"
#include "iox/logging.hpp"
#include "iox/posix_user.hpp"

namespace iox
{
namespace runtime
{
constexpr access_rights SharedMemoryUser::SHM_SEGMENT_PERMISSIONS;

SharedMemoryUser::SharedMemoryUser(const size_t topicSize,
                                   const uint64_t segmentId,
                                   const UntypedRelativePointer::offset_t segmentManagerAddressOffset) noexcept
{
    PosixSharedMemoryObjectBuilder()
        .name(concatenate(iceoryxResourcePrefix(roudi::DEFAULT_UNIQUE_ROUDI_ID, ResourceType::ICEORYX_DEFINED),
                          roudi::SHM_NAME))
        .memorySizeInBytes(topicSize)
        .accessMode(AccessMode::READ_WRITE)
        .openMode(OpenMode::OPEN_EXISTING)
        .permissions(SHM_SEGMENT_PERMISSIONS)
        .create()
        .and_then([this, segmentId, segmentManagerAddressOffset](auto& sharedMemoryObject) {
            auto registeredSuccessfully = UntypedRelativePointer::registerPtrWithId(
                segment_id_t{segmentId},
                sharedMemoryObject.getBaseAddress(),
                sharedMemoryObject.get_size().expect("Failed to acquire SHM size."));

            if (!registeredSuccessfully)
            {
                IOX_REPORT_FATAL(PoshError::POSH__SHM_APP_COULD_NOT_REGISTER_PTR_WITH_GIVEN_SEGMENT_ID);
            }

            IOX_LOG(DEBUG,
                    "Application registered management segment "
                        << iox::log::hex(sharedMemoryObject.getBaseAddress()) << " with size "
                        << sharedMemoryObject.get_size().expect("Failed to acquire SHM size.") << " to id "
                        << segmentId);

            this->openDataSegments(segmentId, segmentManagerAddressOffset);

            m_shmObject.emplace(std::move(sharedMemoryObject));
        })
        .or_else([](auto&) { IOX_REPORT_FATAL(PoshError::POSH__SHM_APP_MAPP_ERR); });
}

SharedMemoryUser::~SharedMemoryUser() noexcept
{
    for (auto& shm : m_dataShmObjects)
    {
        auto segmentId = segment_id_t{UntypedRelativePointer::searchId(shm.getBaseAddress())};
        UntypedRelativePointer::unregisterPtr(segmentId);
    }

    if (m_shmObject.has_value())
    {
        auto segmentId = segment_id_t{UntypedRelativePointer::searchId(m_shmObject->getBaseAddress())};
        UntypedRelativePointer::unregisterPtr(segmentId);
    }
}

void SharedMemoryUser::openDataSegments(const uint64_t segmentId,
                                        const UntypedRelativePointer::offset_t segmentManagerAddressOffset) noexcept
{
    auto* ptr = UntypedRelativePointer::getPtr(segment_id_t{segmentId}, segmentManagerAddressOffset);
    auto* segmentManager = static_cast<mepoo::SegmentManager<>*>(ptr);

    auto segmentMapping = segmentManager->getSegmentMappings(PosixUser::getUserOfCurrentProcess());
    for (const auto& segment : segmentMapping)
    {
        auto accessMode = segment.m_isWritable ? AccessMode::READ_WRITE : AccessMode::READ_ONLY;
        PosixSharedMemoryObjectBuilder()
            .name([&segment] {
                using ShmName_t = detail::PosixSharedMemory::Name_t;
                ShmName_t shmName = iceoryxResourcePrefix(roudi::DEFAULT_UNIQUE_ROUDI_ID, ResourceType::USER_DEFINED);
                if (shmName.size() + segment.m_sharedMemoryName.size() > ShmName_t::capacity())
                {
                    IOX_LOG(FATAL,
                            "The payload segment with the name '"
                                << segment.m_sharedMemoryName.size()
                                << "' would exceed the maximum allowed size when used with the '" << shmName
                                << "' prefix!");
                    IOX_PANIC("");
                }
                shmName.append(TruncateToCapacity, segment.m_sharedMemoryName);
                return shmName;
            }())
            .memorySizeInBytes(segment.m_size)
            .accessMode(accessMode)
            .openMode(OpenMode::OPEN_EXISTING)
            .permissions(SHM_SEGMENT_PERMISSIONS)
            .create()
            .and_then([this, &segment](auto& sharedMemoryObject) {
                if (static_cast<uint32_t>(m_dataShmObjects.size()) >= MAX_SHM_SEGMENTS)
                {
                    IOX_REPORT_FATAL(PoshError::POSH__SHM_APP_SEGMENT_COUNT_OVERFLOW);
                }

                auto registeredSuccessfully = UntypedRelativePointer::registerPtrWithId(
                    segment_id_t{segment.m_segmentId},
                    sharedMemoryObject.getBaseAddress(),
                    sharedMemoryObject.get_size().expect("Failed to get SHM size."));

                if (!registeredSuccessfully)
                {
                    IOX_REPORT_FATAL(PoshError::POSH__SHM_APP_COULD_NOT_REGISTER_PTR_WITH_GIVEN_SEGMENT_ID);
                }

                IOX_LOG(DEBUG,
                        "Application registered payload data segment "
                            << iox::log::hex(sharedMemoryObject.getBaseAddress()) << " with size "
                            << sharedMemoryObject.get_size().expect("Failed to get SHM size.") << " to id "
                            << segment.m_segmentId);

                m_dataShmObjects.emplace_back(std::move(sharedMemoryObject));
            })
            .or_else([](auto&) { IOX_REPORT_FATAL(PoshError::POSH__SHM_APP_SEGMENT_MAPP_ERR); });
    }
}
} // namespace runtime
} // namespace iox
