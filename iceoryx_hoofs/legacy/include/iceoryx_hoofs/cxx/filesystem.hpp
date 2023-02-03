// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_CXX_FILESYSTEM_HPP
#define IOX_HOOFS_CXX_FILESYSTEM_HPP

#include "iox/filesystem.hpp"

namespace iox
{
/// @todo iox-#1593 Deprecate include
/// [[deprecated("Deprecated in 3.0, removed in 4.0, please include 'iox/filesystem.hpp' instead")]]
namespace cxx
{
namespace internal
{
/// @deprecated use `iox::internal::ASCII_A` instead of `iox::cxx::internal::ASCII_A`
using iox::internal::ASCII_A;
/// @deprecated use `iox::internal::ASCII_Z` instead of `iox::cxx::internal::ASCII_Z`
using iox::internal::ASCII_Z;
/// @deprecated use `iox::internal::ASCII_CAPITAL_A` instead of `iox::cxx::internal::ASCII_CAPITAL_A`
using iox::internal::ASCII_CAPITAL_A;
/// @deprecated use `iox::internal::ASCII_CAPITAL_Z` instead of `iox::cxx::internal::ASCII_CAPITAL_Z`
using iox::internal::ASCII_CAPITAL_Z;
/// @deprecated use `iox::internal::ASCII_0` instead of `iox::cxx::internal::ASCII_0`
using iox::internal::ASCII_0;
/// @deprecated use `iox::internal::ASCII_9` instead of `iox::cxx::internal::ASCII_9`
using iox::internal::ASCII_9;
/// @deprecated use `iox::internal::ASCII_MINUS` instead of `iox::cxx::internal::ASCII_MINUS`
using iox::internal::ASCII_MINUS;
/// @deprecated use `iox::internal::ASCII_COLON` instead of `iox::cxx::internal::ASCII_COLON`
using iox::internal::ASCII_COLON;
/// @deprecated use `iox::internal::ASCII_DOT` instead of `iox::cxx::internal::ASCII_DOT`
using iox::internal::ASCII_DOT;
/// @deprecated use `iox::internal::ASCII_UNDERSCORE` instead of `iox::cxx::internal::ASCII_UNDERSCORE`
using iox::internal::ASCII_UNDERSCORE;
} // namespace internal

/// @deprecated use `iox::RelativePathComponents` instead of `iox::cxx::RelativePathComponents`
using iox::RelativePathComponents;

/// @deprecated use `iox::isValidPathEntry` instead of `iox::cxx::isValidPathEntry`
using iox::isValidPathEntry;

/// @deprecated use `iox::isValidFileName` instead of `iox::cxx::isValidFileName`
using iox::isValidFileName;

/// @deprecated use `iox::isValidPathToFile` instead of `iox::cxx::isValidPathToFile`
using iox::isValidPathToFile;

/// @deprecated use `iox::isValidPathToDirectory` instead of `iox::cxx::isValidPathToDirectory`
using iox::isValidPathToDirectory;

/// @deprecated use `iox::doesEndWithPathSeparator` instead of `iox::cxx::doesEndWithPathSeparator`
using iox::doesEndWithPathSeparator;

/// @deprecated use `iox::perms` instead of `iox::cxx::perms`
using iox::perms;
} // namespace cxx
} // namespace iox

#include "iox/detail/filesystem.inl"

#endif
