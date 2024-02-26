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

#ifndef IOX_HOOFS_POSIX_WRAPPER_FILE_LOCK_HPP
#define IOX_HOOFS_POSIX_WRAPPER_FILE_LOCK_HPP

#include "iox/detail/deprecation_marker.hpp"
#include "iox/file_lock.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/file_lock.hpp' instead.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") posix
{
using FileLockError
    IOX_DEPRECATED_SINCE(3, "Please use 'iox::FileLockError' from 'iox/file_lock.hpp' instead.") = FileLockError;
using FileLock IOX_DEPRECATED_SINCE(3, "Please use 'iox::FileLock' from 'iox/file_lock.hpp' instead.") = FileLock;
} // namespace posix
} // namespace iox

// clang-format on

#endif // IOX_HOOFS_POSIX_WRAPPER_FILE_LOCK_HPP
