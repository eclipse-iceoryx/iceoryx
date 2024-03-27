// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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
#include "iox/scope_guard.hpp"

namespace iox
{
namespace runtime
{
constexpr uint32_t SharedMemoryUser::NUMBER_OF_ALL_SHM_SEGMENTS;

expected<SharedMemoryUser, SharedMemoryUserError>
SharedMemoryUser::create(const DomainId domainId,
                         const uint64_t segmentId,
                         const uint64_t managementShmSize,
                         const UntypedRelativePointer::offset_t segmentManagerAddressOffset) noexcept
{
    ShmVector_t shmSegments;
    ScopeGuard shmCleaner{[] {}, [&shmSegments] { SharedMemoryUser::destroy(shmSegments); }};

    // open management segment
    auto shmOpen = openShmSegment(shmSegments,
                                  domainId,
                                  segmentId,
                                  ResourceType::ICEORYX_DEFINED,
                                  {roudi::SHM_NAME},
                                  managementShmSize,
                                  AccessMode::READ_WRITE);
    if (shmOpen.has_error())
    {
        return err(shmOpen.error());
    }

    // open payload segments
    auto* ptr = UntypedRelativePointer::getPtr(segment_id_t{segmentId}, segmentManagerAddressOffset);
    auto* segmentManager = static_cast<mepoo::SegmentManager<>*>(ptr);

    auto segmentMapping = segmentManager->getSegmentMappings(PosixUser::getUserOfCurrentProcess());
    for (const auto& segment : segmentMapping)
    {
        if (static_cast<uint32_t>(shmSegments.size()) >= MAX_SHM_SEGMENTS)
        {
            return err(SharedMemoryUserError::TOO_MANY_SHM_SEGMENTS);
        }

        auto shmOpen = openShmSegment(shmSegments,
                                      domainId,
                                      segment.m_segmentId,
                                      ResourceType::USER_DEFINED,
                                      segment.m_sharedMemoryName,
                                      segment.m_size,
                                      segment.m_isWritable ? AccessMode::READ_WRITE : AccessMode::READ_ONLY);
        if (shmOpen.has_error())
        {
            return err(shmOpen.error());
        }
    }

    ScopeGuard::release(std::move(shmCleaner));
    return ok(SharedMemoryUser{std::move(shmSegments)});
}

SharedMemoryUser::SharedMemoryUser(ShmVector_t&& payloadShm) noexcept
    : m_shmSegments(std::move(payloadShm))
{
}

SharedMemoryUser::~SharedMemoryUser() noexcept
{
    SharedMemoryUser::destroy(m_shmSegments);
}

void SharedMemoryUser::destroy(ShmVector_t& shmSegments) noexcept
{
    while (!shmSegments.empty())
    {
        auto& shm = shmSegments.back();
        auto segmentId = segment_id_t{UntypedRelativePointer::searchId(shm.getBaseAddress())};
        UntypedRelativePointer::unregisterPtr(segmentId);
        shmSegments.pop_back();
    }
}

expected<void, SharedMemoryUserError> SharedMemoryUser::openShmSegment(ShmVector_t& shmSegments,
                                                                       const DomainId domainId,
                                                                       const uint64_t segmentId,
                                                                       const ResourceType resourceType,
                                                                       const ShmName_t& shmName,
                                                                       const uint64_t shmSize,
                                                                       const AccessMode accessMode) noexcept
{
    auto shmResult = PosixSharedMemoryObjectBuilder()
                         .name(concatenate(iceoryxResourcePrefix(domainId, resourceType), shmName))
                         .memorySizeInBytes(shmSize)
                         .accessMode(accessMode)
                         .openMode(OpenMode::OPEN_EXISTING)
                         .create();

    if (shmResult.has_error())
    {
        return err(SharedMemoryUserError::SHM_MAPPING_ERROR);
    }

    auto& shm = shmResult.value();
    auto registeredSuccessfully = UntypedRelativePointer::registerPtrWithId(
        segment_id_t{segmentId}, shm.getBaseAddress(), shm.get_size().expect("Failed to acquire SHM size."));

    if (!registeredSuccessfully)
    {
        return err(SharedMemoryUserError::RELATIVE_POINTER_MAPPING_ERROR);
    }

    IOX_LOG(
        DEBUG,
        "Application registered " << ((resourceType == ResourceType::ICEORYX_DEFINED) ? "management" : "payload data")
                                  << " segment " << iox::log::hex(shm.getBaseAddress()) << " with size "
                                  << shm.get_size().expect("Failed to acquire SHM size.") << " to id " << segmentId);

    shmSegments.emplace_back(std::move(shm));

    return ok();
}
} // namespace runtime
} // namespace iox
