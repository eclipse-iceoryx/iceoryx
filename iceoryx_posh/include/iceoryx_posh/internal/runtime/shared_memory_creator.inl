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

namespace iox
{
namespace runtime
{
template <typename ShmType>
inline SharedMemoryCreator<ShmType>::SharedMemoryCreator(const RouDiConfig_t& config) noexcept
{
    /// @todo these are internal mempool for the introspection, move to a better place
    mepoo::MePooConfig mempoolConfig;
    mempoolConfig.m_mempoolConfig.push_back(
        {static_cast<uint32_t>(cxx::align(sizeof(roudi::MemPoolIntrospectionTopic), 32ul)), 250});
    mempoolConfig.m_mempoolConfig.push_back(
        {static_cast<uint32_t>(cxx::align(sizeof(roudi::ProcessIntrospectionFieldTopic), 32ul)), 10});
    mempoolConfig.m_mempoolConfig.push_back(
        {static_cast<uint32_t>(cxx::align(sizeof(roudi::PortIntrospectionFieldTopic), 32ul)), 10});
    mempoolConfig.m_mempoolConfig.push_back(
        {static_cast<uint32_t>(cxx::align(sizeof(roudi::PortThroughputIntrospectionFieldTopic), 32ul)), 10});
    mempoolConfig.optimize();

    uint64_t totalSharedMemorySize = ShmType::getRequiredSharedMemory()
                                     + mepoo::SegmentManager<>::requiredManagementMemorySize(config)
                                     + mepoo::MemoryManager::requiredFullMemorySize(mempoolConfig);

    auto pageSize = posix::pageSize().value_or(posix::MaxPageSize);
    auto alignedBaseAddressHint = cxx::align(config.roudi.m_sharedMemoryBaseAddressOffset, pageSize);
    if (alignedBaseAddressHint != config.roudi.m_sharedMemoryBaseAddressOffset)
    {
        LogWarn() << "The SHM start address hint (" << log::HexFormat(config.roudi.m_sharedMemoryBaseAddressOffset)
                  << ") is not aligned to the page size (" << log::HexFormat(pageSize) << ") hint will be set to "
                  << log::HexFormat(alignedBaseAddressHint);
    }

    // create and map a shared memory region
    m_shmObject = posix::SharedMemoryObject::create(SHM_NAME,
                                                    totalSharedMemorySize,
                                                    posix::AccessMode::readWrite,
                                                    posix::OwnerShip::mine,
                                                    reinterpret_cast<void*>(alignedBaseAddressHint));

    if (!m_shmObject.has_value())
    {
        errorHandler(Error::kPOSH__SHM_BAD_ALLOC);
    }

    if (nullptr == m_shmObject->getBaseAddress())
    {
        errorHandler(Error::kPOSH__SHM_ROUDI_MAPP_ERR);
    }

    if (config.roudi.m_verifySharedMemoryPlacement
        && reinterpret_cast<uintptr_t>(m_shmObject->getBaseAddress()) != alignedBaseAddressHint)
    {
        LogWarn() << "RouDi mapped the shared memory on a different position then requested. This can cause "
                  << "problems on application side! If you are using the address sanitizer this behavior "
                  << "is expected but we currently do not support the address sanitizer";
    }

    // now construct our POSH shared memory object
    m_shmTypePtr = static_cast<ShmType*>(m_shmObject->allocate(sizeof(ShmType)));
    m_shmTypePtr = new (m_shmTypePtr) ShmType(
        m_shmObject->getAllocator(),
        config,
        cxx::align(reinterpret_cast<uintptr_t>(m_shmObject->getBaseAddress()) + totalSharedMemorySize, pageSize),
        config.roudi.m_verifySharedMemoryPlacement);

    m_shmTypePtr->m_roudiMemoryManager.configureMemoryManager(
        mempoolConfig, m_shmObject->getAllocator(), m_shmObject->getAllocator());
    m_shmObject->finalizeAllocation();
}

template <typename ShmType>
inline SharedMemoryCreator<ShmType>::~SharedMemoryCreator() noexcept
{
    if (m_shmTypePtr)
    {
        m_shmTypePtr->~ShmType();
    }
}

template <typename ShmType>
inline std::string SharedMemoryCreator<ShmType>::getBaseAddrString() const noexcept
{
    size_t ptr = reinterpret_cast<size_t>(m_shmObject->getBaseAddress());
    return std::to_string(ptr);
}

template <typename ShmType>
inline uint64_t SharedMemoryCreator<ShmType>::getShmSizeInBytes() const noexcept
{
    return m_shmObject->getSizeInBytes();
}

template <typename ShmType>
inline ShmType* SharedMemoryCreator<ShmType>::getShmInterface() const noexcept
{
    return m_shmTypePtr;
}

} // namespace runtime
} // namespace iox
