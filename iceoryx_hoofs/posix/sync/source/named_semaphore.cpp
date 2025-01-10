// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iox/named_semaphore.hpp"
#include "iox/detail/semaphore_helper.hpp"
#include "iox/logging.hpp"
#include "iox/posix_call.hpp"

namespace iox
{
static string<NamedSemaphore::Name_t::capacity() + 1> createNameWithSlash(const NamedSemaphore::Name_t& name)
{
    string<platform::IOX_MAX_SEMAPHORE_NAME_LENGTH> nameWithSlash = name;
    nameWithSlash.insert(0, "/", 1);
    return nameWithSlash;
}

static expected<void, SemaphoreError> unlink(const NamedSemaphore::Name_t& name) noexcept
{
    auto result = IOX_POSIX_CALL(iox_sem_unlink)(createNameWithSlash(name).c_str())
                      .failureReturnValue(-1)
                      .ignoreErrnos(ENOENT)
                      .evaluate();
    if (result.has_error())
    {
        switch (result.error().errnum)
        {
        case EACCES:
            IOX_LOG(Error, "You don't have permission to remove the semaphore \"" << name << '"');
            return err(SemaphoreError::PERMISSION_DENIED);
        default:
            IOX_LOG(Error,
                    "This should never happen. An unknown error occurred while creating the semaphore \"" << name
                                                                                                          << '"');
            return err(SemaphoreError::UNDEFINED);
        }
    }
    return ok();
}

static expected<bool, SemaphoreError>
// NOLINTJUSTIFICATION the cognitive complexity results from the expanded log macro
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
tryOpenExistingSemaphore(optional<NamedSemaphore>& uninitializedSemaphore, const NamedSemaphore::Name_t& name) noexcept
{
    auto result = IOX_POSIX_CALL(iox_sem_open)(createNameWithSlash(name).c_str(), 0)
                      .failureReturnValue(IOX_SEM_FAILED)
                      .ignoreErrnos(ENOENT)
                      .evaluate();

    if (result.has_error())
    {
        switch (result.error().errnum)
        {
        case EACCES:
            IOX_LOG(Error, "Insufficient permissions to open semaphore \"" << name << '"');
            return err(SemaphoreError::PERMISSION_DENIED);
        case EMFILE:
            IOX_LOG(Error,
                    "The per-process limit of file descriptor exceeded while opening the semaphore \"" << name << '"');
            return err(SemaphoreError::FILE_DESCRIPTOR_LIMIT_REACHED);
        case ENFILE:
            IOX_LOG(Error,
                    "The system wide limit of file descriptor exceeded while opening the semaphore \"" << name << '"');
            return err(SemaphoreError::FILE_DESCRIPTOR_LIMIT_REACHED);
        case ENOMEM:
            IOX_LOG(Error, "Insufficient memory to open the semaphore \"" << name << '"');
            return err(SemaphoreError::OUT_OF_MEMORY);
        default:
            IOX_LOG(Error,
                    "This should never happen. An unknown error occurred while opening the semaphore \"" << name
                                                                                                         << '"');
            return err(SemaphoreError::UNDEFINED);
        }
    }

    if (result->errnum != ENOENT)
    {
        constexpr bool HAS_OWNERSHIP = false;
        uninitializedSemaphore.emplace(result.value().value, name, HAS_OWNERSHIP);
        return ok(true);
    }

    return ok(false);
}

// NOLINTJUSTIFICATION used only internally in this file. Furthermore the problem cannot be avoided since
//                     those arguments are required by the posix function sem_open. One could call it
//                     before this function and provide the result but this would increase code complexity
//                     even further. The cognitive complexity results from the expanded log macro
// NOLINTNEXTLINE(readability-function-size,readability-function-cognitive-complexity)
static expected<void, SemaphoreError> createSemaphore(optional<NamedSemaphore>& uninitializedSemaphore,
                                                      const NamedSemaphore::Name_t& name,
                                                      const OpenMode openMode,
                                                      const access_rights permissions,
                                                      const uint32_t initialValue) noexcept
{
    auto result = IOX_POSIX_CALL(iox_sem_open_ext)(createNameWithSlash(name).c_str(),
                                                   convertToOflags(openMode),
                                                   permissions.value(),
                                                   static_cast<unsigned int>(initialValue))
                      .failureReturnValue(IOX_SEM_FAILED)
                      .evaluate();

    if (result.has_error())
    {
        switch (result.error().errnum)
        {
        case EACCES:
            IOX_LOG(Error, "Insufficient permissions to create semaphore \"" << name << '"');
            return err(SemaphoreError::PERMISSION_DENIED);
        case EEXIST:
            IOX_LOG(Error,
                    "A semaphore with the name \""
                        << name
                        << "\" does already exist. This should not happen until there is a race condition when "
                           "multiple instances try to create the same named semaphore concurrently.");
            return err(SemaphoreError::ALREADY_EXIST);
        case EMFILE:
            IOX_LOG(Error,
                    "The per-process limit of file descriptor exceeded while creating the semaphore \"" << name << '"');
            return err(SemaphoreError::FILE_DESCRIPTOR_LIMIT_REACHED);
        case ENFILE:
            IOX_LOG(Error,
                    "The system wide limit of file descriptor exceeded while creating the semaphore \"" << name << '"');
            return err(SemaphoreError::FILE_DESCRIPTOR_LIMIT_REACHED);
        case ENOMEM:
            IOX_LOG(Error, "Insufficient memory to create the semaphore \"" << name << '"');
            return err(SemaphoreError::OUT_OF_MEMORY);
        default:
            IOX_LOG(Error,
                    "This should never happen. An unknown error occurred while creating the semaphore \"" << name
                                                                                                          << '"');
            return err(SemaphoreError::UNDEFINED);
        }
    }

    constexpr bool HAS_OWNERSHIP = true;
    uninitializedSemaphore.emplace(result.value().value, name, HAS_OWNERSHIP);

    return ok();
}

expected<void, SemaphoreError>
// NOLINTJUSTIFICATION the function size is related to the error handling and the cognitive complexity
// results from the expanded log macro
// NOLINTNEXTLINE(readability-function-size,readability-function-cognitive-complexity)
NamedSemaphoreBuilder::create(optional<NamedSemaphore>& uninitializedSemaphore) const noexcept
{
    if (!isValidFileName(m_name))
    {
        IOX_LOG(Error, "The name \"" << m_name << "\" is not a valid semaphore name.");
        return err(SemaphoreError::INVALID_NAME);
    }

    if (m_initialValue > IOX_SEM_VALUE_MAX)
    {
        IOX_LOG(Error,
                "The semaphores \"" << m_name << "\" initial value of " << m_initialValue
                                    << " exceeds the maximum semaphore value " << IOX_SEM_VALUE_MAX);
        return err(SemaphoreError::SEMAPHORE_OVERFLOW);
    }

    if (m_openMode == OpenMode::OpenExisting)
    {
        auto result = tryOpenExistingSemaphore(uninitializedSemaphore, m_name);
        if (result.has_error())
        {
            return result;
        }

        if (!result.value())
        {
            IOX_LOG(Error, "Unable to open semaphore since no semaphore with the name \"" << m_name << "\" exists.");
            return err(SemaphoreError::NO_SEMAPHORE_WITH_THAT_NAME_EXISTS);
        }
        return ok();
    }

    if (m_openMode == OpenMode::OpenOrCreate)
    {
        auto result = tryOpenExistingSemaphore(uninitializedSemaphore, m_name);
        if (result.has_error())
        {
            return result;
        }

        if (result.value())
        {
            return ok();
        }

        return createSemaphore(uninitializedSemaphore, m_name, m_openMode, m_permissions, m_initialValue);
    }

    if (m_openMode == OpenMode::ExclusiveCreate)
    {
        return createSemaphore(uninitializedSemaphore, m_name, m_openMode, m_permissions, m_initialValue);
    }

    // if OpenMode::PurgeAndCreate, written outside of if statement to avoid no return value warning
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
    if (IOX_POSIX_CALL(iox_sem_close)(m_handle).failureReturnValue(-1).evaluate().has_error())
    {
        IOX_LOG(Error, "This should never happen. Unable to close named semaphore \"" << m_name << '"');
    }

    if (m_hasOwnership)
    {
        IOX_DISCARD_RESULT(unlink(m_name));
    }
}

expected<void, SemaphoreError> NamedSemaphore::post_impl() noexcept
{
    return detail::sem_post(m_handle);
}
expected<void, SemaphoreError> NamedSemaphore::wait_impl() noexcept
{
    return detail::sem_wait(m_handle);
}
expected<bool, SemaphoreError> NamedSemaphore::try_wait_impl() noexcept
{
    return detail::sem_try_wait(m_handle);
}
expected<SemaphoreWaitState, SemaphoreError> NamedSemaphore::timed_wait_impl(const units::Duration& timeout) noexcept
{
    return detail::sem_timed_wait(m_handle, timeout);
}
} // namespace iox
