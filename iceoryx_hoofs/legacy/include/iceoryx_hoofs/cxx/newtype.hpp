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
#ifndef IOX_HOOFS_CXX_NEWTYPE_HPP
#define IOX_HOOFS_CXX_NEWTYPE_HPP

#include "iox/detail/deprecation_marker.hpp"
#include "iox/newtype.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/newtype.hpp' instead.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") cxx
{
/// @deprecated use 'iox::NewType' instead of 'iox::cxx::NewType'
using iox::NewType;

namespace newtype
{
/// @deprecated use 'iox::newtype::AssignByValueCopy' instead of 'iox::cxx::newtype::AssignByValueCopy'
using iox::newtype::AssignByValueCopy;
/// @deprecated use 'iox::newtype::AssignByValueMove' instead of 'iox::cxx::newtype::AssignByValueMove'
using iox::newtype::AssignByValueMove;
/// @deprecated use 'iox::newtype::Comparable' instead of 'iox::cxx::newtype::Comparable'
using iox::newtype::Comparable;
/// @deprecated use 'iox::newtype::ConstructByValueCopy' instead of 'iox::cxx::newtype::ConstructByValueCopy'
using iox::newtype::ConstructByValueCopy;
/// @deprecated use 'iox::newtype::Convertable' instead of 'iox::cxx::newtype::Convertable'
using iox::newtype::Convertable;
/// @deprecated use 'iox::newtype::CopyAssignable' instead of 'iox::cxx::newtype::CopyAssignable'
using iox::newtype::CopyAssignable;
/// @deprecated use 'iox::newtype::CopyConstructable' instead of 'iox::cxx::newtype::CopyConstructable'
using iox::newtype::CopyConstructable;
/// @deprecated use 'iox::newtype::DefaultConstructable' instead of 'iox::cxx::newtype::DefaultConstructable'
using iox::newtype::DefaultConstructable;
/// @deprecated use 'iox::newtype::MoveAssignable' instead of 'iox::cxx::newtype::MoveAssignable'
using iox::newtype::MoveAssignable;
/// @deprecated use 'iox::newtype::MoveConstructable' instead of 'iox::cxx::newtype::MoveConstructable'
using iox::newtype::MoveConstructable;
/// @deprecated use 'iox::newtype::ProtectedConstructByValueCopy' instead of
/// 'iox::cxx::newtype::ProtectedConstructByValueCopy'
using iox::newtype::ProtectedConstructByValueCopy;
/// @deprecated use 'iox::newtype::Sortable' instead of 'iox::cxx::newtype::Sortable'
using iox::newtype::Sortable;

namespace internal
{
using iox::newtype::internal::ProtectedConstructor;
}
} // namespace newtype
} // namespace cxx
} // namespace iox

// clang-format on

#endif
