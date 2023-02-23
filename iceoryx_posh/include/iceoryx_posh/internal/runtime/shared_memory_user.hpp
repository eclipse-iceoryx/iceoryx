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

#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iox/filesystem.hpp"
#include "iox/optional.hpp"
#include "iox/relative_pointer.hpp"
#include "iox/vector.hpp"


namespace iox
{
namespace runtime
{
/// @brief shared memory setup for the management segment user side
class SharedMemoryUser
{
  public:
    /// @brief Constructor
    /// @param[in] topicSize size of the shared memory management segment
    /// @param[in] segmentManagerAddr adress of the segment manager that does the final mapping of memory in the process
    /// @param[in] segmentId of the relocatable shared memory segment
    /// address space
    SharedMemoryUser(const size_t topicSize,
                     const uint64_t segmentId,
                     const UntypedRelativePointer::offset_t segmentManagerAddressOffset) noexcept;

  private:
    void openDataSegments(const uint64_t segmentId,
                          const UntypedRelativePointer::offset_t segmentManagerAddressOffset) noexcept;

  private:
    optional<posix::SharedMemoryObject> m_shmObject;
    vector<posix::SharedMemoryObject, MAX_SHM_SEGMENTS> m_dataShmObjects;
    static constexpr access_rights SHM_SEGMENT_PERMISSIONS =
        perms::owner_read | perms::owner_write | perms::group_read | perms::group_write;
};

} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_SHARED_MEMORY_USER_HPP
