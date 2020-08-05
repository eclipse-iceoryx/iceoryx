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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_HPP
#define IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_HPP

#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/function_ref.hpp"

#include <memory>

namespace iox
{
namespace popo
{

enum class ChunkRecallError : uint8_t
{
    NO_PREVIOUS_CHUNK,
    CHUNK_ALREADY_CLAIMED
};

struct Untyped{};

template<typename T, typename port_t = iox::popo::PublisherPortUser>
class Publisher
{
public:
    using SamplePtr = cxx::unique_ptr<T>;
    using uid_t = uint64_t;

    // Temporary, to be replaced with service description / id based constructors
    Publisher() = default;

    Publisher(const Publisher& other) = delete;
    Publisher& operator=(const Publisher&) = delete;
    Publisher(Publisher&& rhs) = default;
    Publisher& operator=(Publisher&& rhs) = default;
    ~Publisher() = default;

    ///
    /// @brief uid Get the unique id of the publisher.
    /// @return The unique id.
    ///
    uid_t uid() const noexcept;

    ///
    /// @brief allocate Allocates a chunk of shared memory.
    /// @return Pointer to the successfully allocated memory, otherwise an allocation error.
    ///
    cxx::expected<SamplePtr, AllocationError> allocate() const noexcept;

    ///
    /// @brief allocate Allocate a chunk then execute the provided callable using the allocated chunk.
    /// @param f Callable to execute, taking the allocated chunk as its parameter.
    /// @return Success if chunk was successfully allocated and the provided callable was successfully executed.
    ///
    cxx::expected<AllocationError> allocate(cxx::function_ref<void(SamplePtr&) noexcept> f) const noexcept;

    ///
    /// @brief release Releases ownership of an unused allocated chunk.
    /// @details The released chunk will be freed as soon as there are no longer any active references
    /// to it in the system.
    /// @param chunk
    ///
    void release(SamplePtr&& chunk) const noexcept;

    ///
    /// @brief send Publishes the chunk to the system.
    /// @details Ownership of published chunks is automatically released.
    /// @param chunk
    ///
    void publish(SamplePtr&& chunk) const noexcept;

    ///
    /// @brief copyAndPublish Copy the given sample into a shared memory chunk and immediately publish.
    /// @details This method should not be used for larger data types as it includes a copy. For larger data types, it
    /// is preferred to first allocate a chunk and then directly write the data into it (e.g. with a placement new),
    /// rather than to write it elsewhere then copy it in.
    /// @param val The value to publish.
    ///
    void publishCopyOf(const T& val) const noexcept;

    ///
    /// @brief previous Reclaims ownership of a previously published chunk if it has not yet been accessed.
    /// @return The previously published chunk if one exists and is unclaimed, otherwise an error.
    ///
    cxx::expected<ChunkRecallError> previous() const noexcept;

    void offer() noexcept;
    void stopOffer() noexcept;
    bool isOffered() const noexcept;
    bool hasSubscribers() const noexcept;

protected:
    port_t m_sender{nullptr};
    bool m_useDynamicPayloadSize = true;

private:


};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/experimental/internal/popo/publisher.inl"

#endif // IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_HPP
