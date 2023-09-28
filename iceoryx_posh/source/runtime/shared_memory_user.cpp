// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_posh/error_handling/error_handling.hpp"
#include "iceoryx_posh/internal/mepoo/segment_manager.hpp"
#include "iox/logging.hpp"

namespace iox
{
namespace runtime
{
constexpr access_rights SharedMemoryUser::SHM_SEGMENT_PERMISSIONS;

SharedMemoryUser::SharedMemoryUser(const size_t topicSize,
                                   const uint64_t segmentId,
                                   const UntypedRelativePointer::offset_t segmentManagerAddressOffset) noexcept
{
    posix::SharedMemoryObjectBuilder()
        .name(roudi::SHM_NAME)
        .memorySizeInBytes(topicSize)
        .accessMode(posix::AccessMode::READ_WRITE)
        .openMode(posix::OpenMode::OPEN_EXISTING)
        .permissions(SHM_SEGMENT_PERMISSIONS)
        .create()
        .and_then([this, segmentId, segmentManagerAddressOffset](auto& sharedMemoryObject) {
            auto registeredSuccessfully = UntypedRelativePointer::registerPtrWithId(
                segment_id_t{segmentId},
                sharedMemoryObject.getBaseAddress(),
                sharedMemoryObject.get_size().expect("Failed to acquire SHM size."));

            if (!registeredSuccessfully)
            {
                errorHandler(PoshError::POSH__SHM_APP_COULD_NOT_REGISTER_PTR_WITH_GIVEN_SEGMENT_ID);
            }

            IOX_LOG(DEBUG,
                    "Application registered management segment "
                        << iox::log::hex(sharedMemoryObject.getBaseAddress()) << " with size "
                        << sharedMemoryObject.get_size().expect("Failed to acquire SHM size.") << " to id "
                        << segmentId);

            this->openDataSegments(segmentId, segmentManagerAddressOffset);

            m_shmObject.emplace(std::move(sharedMemoryObject));
        })
        .or_else([](auto&) { errorHandler(PoshError::POSH__SHM_APP_MAPP_ERR); });
}

void SharedMemoryUser::openDataSegments(const uint64_t segmentId,
                                        const UntypedRelativePointer::offset_t segmentManagerAddressOffset) noexcept
{
    auto* ptr = UntypedRelativePointer::getPtr(segment_id_t{segmentId}, segmentManagerAddressOffset);
    auto* segmentManager = static_cast<mepoo::SegmentManager<>*>(ptr);

    auto segmentMapping = segmentManager->getSegmentMappings(posix::PosixUser::getUserOfCurrentProcess());
    for (const auto& segment : segmentMapping)
    {
        auto accessMode = segment.m_isWritable ? posix::AccessMode::READ_WRITE : posix::AccessMode::READ_ONLY;
        posix::SharedMemoryObjectBuilder()
            .name(segment.m_sharedMemoryName)
            .memorySizeInBytes(segment.m_size)
            .accessMode(accessMode)
            .openMode(posix::OpenMode::OPEN_EXISTING)
            .permissions(SHM_SEGMENT_PERMISSIONS)
            .create()
            .and_then([this, &segment](auto& sharedMemoryObject) {
                if (static_cast<uint32_t>(m_dataShmObjects.size()) >= MAX_SHM_SEGMENTS)
                {
                    errorHandler(PoshError::POSH__SHM_APP_SEGMENT_COUNT_OVERFLOW);
                }

                auto registeredSuccessfully = UntypedRelativePointer::registerPtrWithId(
                    segment_id_t{segment.m_segmentId},
                    sharedMemoryObject.getBaseAddress(),
                    sharedMemoryObject.get_size().expect("Failed to get SHM size."));

                if (!registeredSuccessfully)
                {
                    errorHandler(PoshError::POSH__SHM_APP_COULD_NOT_REGISTER_PTR_WITH_GIVEN_SEGMENT_ID);
                }

                IOX_LOG(DEBUG,
                        "Application registered payload data segment "
                            << iox::log::hex(sharedMemoryObject.getBaseAddress()) << " with size "
                            << sharedMemoryObject.get_size().expect("Failed to get SHM size.") << " to id "
                            << segment.m_segmentId);

                m_dataShmObjects.emplace_back(std::move(sharedMemoryObject));
            })
            .or_else([](auto&) { errorHandler(PoshError::POSH__SHM_APP_SEGMENT_MAPP_ERR); });
    }
}
} // namespace runtime
} // namespace iox
