// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/posix_wrapper/named_semaphore.hpp"
#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

namespace iox
{
namespace posix
{
static cxx::string<NamedSemaphore::Name_t::capacity() + 1> createNameWithSlash(const NamedSemaphore::Name_t& name)
{
    cxx::string<platform::IOX_MAX_SEMAPHORE_NAME_LENGTH> nameWithSlash = name;
    nameWithSlash.insert(0, "/", 1);
    return nameWithSlash;
}

static cxx::expected<SemaphoreError> unlink(const NamedSemaphore::Name_t& name) noexcept
{
    auto result = posixCall(iox_sem_unlink)(createNameWithSlash(name).c_str())
                      .failureReturnValue(-1)
                      .ignoreErrnos(ENOENT)
                      .evaluate();
    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EACCES:
            LogError() << "You don't have permission to remove the semaphore \"" << name << "\"";
            return cxx::error<SemaphoreError>(SemaphoreError::PERMISSION_DENIED);
        default:
            LogError() << "This should never happen. An unknown error occurred while creating the semaphore \"" << name
                       << "\".";
            return cxx::error<SemaphoreError>(SemaphoreError::UNDEFINED);
        }
    }
    return cxx::success<>();
}

static cxx::expected<bool, SemaphoreError>
// NOLINTJUSTIFICATION the cognitive complexity results from the expanded log macro
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
tryOpenExistingSemaphore(cxx::optional<NamedSemaphore>& uninitializedSemaphore,
                         const NamedSemaphore::Name_t& name) noexcept
{
    auto result = posixCall(iox_sem_open)(createNameWithSlash(name).c_str(), 0)
                      .failureReturnValue(IOX_SEM_FAILED)
                      .ignoreErrnos(ENOENT)
                      .evaluate();

    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EACCES:
            LogError() << "Insufficient permissions to open semaphore \"" << name << "\".";
            return cxx::error<SemaphoreError>(SemaphoreError::PERMISSION_DENIED);
        case EMFILE:
            LogError() << "The per-process limit of file descriptor exceeded while opening the semaphore \"" << name
                       << "\"";
            return cxx::error<SemaphoreError>(SemaphoreError::FILE_DESCRIPTOR_LIMIT_REACHED);
        case ENFILE:
            LogError() << "The system wide limit of file descriptor exceeded while opening the semaphore \"" << name
                       << "\"";
            return cxx::error<SemaphoreError>(SemaphoreError::FILE_DESCRIPTOR_LIMIT_REACHED);
        case ENOMEM:
            LogError() << "Insufficient memory to open the semaphore \"" << name << "\".";
            return cxx::error<SemaphoreError>(SemaphoreError::OUT_OF_MEMORY);
        default:
            LogError() << "This should never happen. An unknown error occurred while opening the semaphore \"" << name
                       << "\".";
            return cxx::error<SemaphoreError>(SemaphoreError::UNDEFINED);
        }
    }

    if (result->errnum != ENOENT)
    {
        constexpr bool HAS_OWNERSHIP = false;
        uninitializedSemaphore.emplace(result.value().value, name, HAS_OWNERSHIP);
        return cxx::success<bool>(true);
    }

    return cxx::success<bool>(false);
}

/// NOLINTJUSTIFICATION used only internally in this file. Furthermore the problem cannot be avoided since
///                     those arguments are required by the posix function sem_open. One could call it
///                     before this function and provide the result but this would increase code complexity
///                     even further. The cognitive complexity results from the expanded log macro
/// NOLINTNEXTLINE(readability-function-size,readability-function-cognitive-complexity)
static cxx::expected<SemaphoreError> createSemaphore(cxx::optional<NamedSemaphore>& uninitializedSemaphore,
                                                     const NamedSemaphore::Name_t& name,
                                                     const OpenMode openMode,
                                                     const cxx::perms permissions,
                                                     const uint32_t initialValue) noexcept
{
    auto result = posixCall(iox_sem_open_ext)(createNameWithSlash(name).c_str(),
                                              convertToOflags(openMode),
                                              static_cast<mode_t>(permissions),
                                              static_cast<unsigned int>(initialValue))
                      .failureReturnValue(IOX_SEM_FAILED)
                      .evaluate();

    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EACCES:
            LogError() << "Insufficient permissions to create semaphore \"" << name << "\".";
            return cxx::error<SemaphoreError>(SemaphoreError::PERMISSION_DENIED);
        case EEXIST:
            LogError() << "A semaphore with the name \"" << name
                       << "\" does already exist. This should not happen until there is a race condition when multiple "
                          "instances try to create the same named semaphore concurrently.";
            return cxx::error<SemaphoreError>(SemaphoreError::ALREADY_EXIST);
        case EMFILE:
            LogError() << "The per-process limit of file descriptor exceeded while creating the semaphore \"" << name
                       << "\"";
            return cxx::error<SemaphoreError>(SemaphoreError::FILE_DESCRIPTOR_LIMIT_REACHED);
        case ENFILE:
            LogError() << "The system wide limit of file descriptor exceeded while creating the semaphore \"" << name
                       << "\"";
            return cxx::error<SemaphoreError>(SemaphoreError::FILE_DESCRIPTOR_LIMIT_REACHED);
        case ENOMEM:
            LogError() << "Insufficient memory to create the semaphore \"" << name << "\".";
            return cxx::error<SemaphoreError>(SemaphoreError::OUT_OF_MEMORY);
        default:
            LogError() << "This should never happen. An unknown error occurred while creating the semaphore \"" << name
                       << "\".";
            return cxx::error<SemaphoreError>(SemaphoreError::UNDEFINED);
        }
    }

    constexpr bool HAS_OWNERSHIP = true;
    uninitializedSemaphore.emplace(result.value().value, name, HAS_OWNERSHIP);

    return cxx::success<>();
}

cxx::expected<SemaphoreError>
// NOLINTJUSTIFICATION the function size is related to the error handling and the cognitive complexity
// results from the expanded log macro
// NOLINTNEXTLINE(readability-function-size,readability-function-cognitive-complexity)
NamedSemaphoreBuilder::create(cxx::optional<NamedSemaphore>& uninitializedSemaphore) const noexcept
{
    if (!cxx::isValidFileName(m_name))
    {
        LogError() << "The name \"" << m_name << "\" is not a valid semaphore name.";
        return cxx::error<SemaphoreError>(SemaphoreError::INVALID_NAME);
    }

    if (m_initialValue > IOX_SEM_VALUE_MAX)
    {
        LogError() << "The semaphores \"" << m_name << "\" initial value of " << m_initialValue
                   << " exceeds the maximum semaphore value " << IOX_SEM_VALUE_MAX;
        return cxx::error<SemaphoreError>(SemaphoreError::SEMAPHORE_OVERFLOW);
    }

    if (m_openMode == OpenMode::OPEN_EXISTING)
    {
        auto result = tryOpenExistingSemaphore(uninitializedSemaphore, m_name);
        if (result.has_error())
        {
            return result;
        }

        if (!result.value())
        {
            LogError() << "Unable to open semaphore since no semaphore with the name \"" << m_name << "\" exists.";
            return cxx::error<SemaphoreError>(SemaphoreError::NO_SEMAPHORE_WITH_THAT_NAME_EXISTS);
        }
        return cxx::success<>();
    }

    if (m_openMode == OpenMode::OPEN_OR_CREATE)
    {
        auto result = tryOpenExistingSemaphore(uninitializedSemaphore, m_name);
        if (result.has_error())
        {
            return result;
        }

        if (result.value())
        {
            return cxx::success<>();
        }

        return createSemaphore(uninitializedSemaphore, m_name, m_openMode, m_permissions, m_initialValue);
    }

    if (m_openMode == OpenMode::EXCLUSIVE_CREATE)
    {
        return createSemaphore(uninitializedSemaphore, m_name, m_openMode, m_permissions, m_initialValue);
    }

    // if OpenMode::PURGE_AND_CREATE, written outside of if statement to avoid no return value warning
    auto result = unlink(m_name);
    if (result.has_error())
    {
        return result;
    }
    return createSemaphore(uninitializedSemaphore, m_name, m_openMode, m_permissions, m_initialValue);
}

NamedSemaphore::NamedSemaphore(iox_sem_t* handle, const Name_t& name, const bool hasOwnership) noexcept
    : m_handle{handle}
    , m_name{name}
    , m_hasOwnership{hasOwnership}
{
}

NamedSemaphore::~NamedSemaphore() noexcept
{
    if (posixCall(iox_sem_close)(m_handle).failureReturnValue(-1).evaluate().has_error())
    {
        LogError() << "This should never happen. Unable to close named semaphore \"" << m_name << "\"";
    }

    if (m_hasOwnership)
    {
        IOX_DISCARD_RESULT(unlink(m_name));
    }
}

iox_sem_t* NamedSemaphore::getHandle() noexcept
{
    return m_handle;
}
} // namespace posix
} // namespace iox
