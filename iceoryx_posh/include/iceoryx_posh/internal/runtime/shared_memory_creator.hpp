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

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/mepoo/segment_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

#include <cstdint>
#include <string>

namespace iox
{
namespace runtime
{
/// @brief Creates the shared memory based on a provided configuration
template <typename ShmType>
class SharedMemoryCreator
{
  public:
    /// @brief Constructor for shared memory object
    /// @param[in] config configuration for things to create in shared memory
    SharedMemoryCreator(const RouDiConfig_t& config) noexcept;

    /// @brief Destructor for shared memory object
    ~SharedMemoryCreator() noexcept;

    SharedMemoryCreator(const SharedMemoryCreator&) = delete;
    SharedMemoryCreator(SharedMemoryCreator&&) = delete;
    SharedMemoryCreator& operator=(const SharedMemoryCreator&) = delete;
    SharedMemoryCreator& operator=(SharedMemoryCreator&&) = delete;

    /// @brief Get function for base adress of shared memory
    /// @return std::string base adress of shared memory
    std::string getBaseAddrString() const noexcept;

    /// @brief Get function for shared memory object size
    /// @return size of the shared memory object
    uint64_t getShmSizeInBytes() const noexcept;

    /// @brief Get function for the shared memory object
    /// @return pointer to the object created in the shared memory
    ShmType* getShmInterface() const noexcept;

    /// @brief Get the segment id of the shared memory object
    /// @return segment id of the shared memory object
    uint64_t getSegmentId() const noexcept;

  private:
    cxx::optional<posix::SharedMemoryObject> m_shmObject;
    ShmType* m_shmTypePtr{nullptr};
};

} // namespace runtime
} // namespace iox

#include "iceoryx_posh/internal/runtime/shared_memory_creator.inl"
