// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_RUNTIME_SHARED_MEMORY_USER_HPP
#define IOX_POSH_RUNTIME_SHARED_MEMORY_USER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iox/builder.hpp"
#include "iox/filesystem.hpp"
#include "iox/optional.hpp"
#include "iox/posix_shared_memory_object.hpp"
#include "iox/relative_pointer.hpp"
#include "iox/vector.hpp"

namespace iox
{
namespace runtime
{

enum class SharedMemoryUserError
{
    SHM_MAPPING_ERROR,
    RELATIVE_POINTER_MAPPING_ERROR,
    TOO_MANY_SHM_SEGMENTS,
};

/// @brief shared memory setup for the management segment user side
class SharedMemoryUser
{
  private:
    static constexpr uint32_t NUMBER_OF_ALL_SHM_SEGMENTS{1 /* management shm */ + MAX_SHM_SEGMENTS /* payload shm */};
    using ShmVector_t = vector<PosixSharedMemoryObject, NUMBER_OF_ALL_SHM_SEGMENTS>;

  public:
    /// @brief Creates a 'SharedMemoryUser'
    /// @param[in] domainId to tie the shared memory to
    /// @param[in] segmentId of the segment for the relocatable shared memory segment address space
    /// @param[in] managementShmSize size of the shared memory management segment
    /// @param[in] segmentManagerAddressOffset adress of the segment manager that does the final mapping of memory in
    /// the process
    /// @return a 'SharedMemoryUser' instance or an 'SharedMemoryUserError' on failure
    static expected<SharedMemoryUser, SharedMemoryUserError>
    create(const DomainId domainId,
           const uint64_t segmentId,
           const uint64_t managementShmSize,
           const UntypedRelativePointer::offset_t segmentManagerAddressOffset) noexcept;

    ~SharedMemoryUser() noexcept;

    SharedMemoryUser(SharedMemoryUser&&) noexcept = default;
    SharedMemoryUser& operator=(SharedMemoryUser&&) noexcept = default;

    SharedMemoryUser(const SharedMemoryUser&) = delete;
    SharedMemoryUser& operator=(const SharedMemoryUser&) = delete;

  private:
    SharedMemoryUser(ShmVector_t&& payloadShm) noexcept;

    static void destroy(ShmVector_t& shmSegments) noexcept;

    static expected<void, SharedMemoryUserError> openShmSegment(ShmVector_t& shmSegments,
                                                                const DomainId domainId,
                                                                const uint64_t segmentId,
                                                                const ResourceType resourceType,
                                                                const ShmName_t& shmName,
                                                                const uint64_t shmSize,
                                                                const AccessMode accessMode) noexcept;


  private:
    ShmVector_t m_shmSegments;
};

} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_SHARED_MEMORY_USER_HPP
