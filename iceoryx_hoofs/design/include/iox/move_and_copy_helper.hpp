// Copyright (c) 2023 by Dennis Liu <dennis48161025@gmail.com>. All rights reserved. reserved.
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

#ifndef IOX_HOOFS_DESIGN_MOVE_AND_COPY_HELPER_HPP
#define IOX_HOOFS_DESIGN_MOVE_AND_COPY_HELPER_HPP

#include <functional>
#include <utility>

namespace iox
{

enum class MoveAndCopyOperations
{
    CopyConstructor,
    CopyAssignment,
    MoveConstructor,
    MoveAssignment,
};

template <MoveAndCopyOperations Opt>
struct MoveAndCopyHelper;

} // namespace iox

#include "iox/detail/move_and_copy_helper.inl"

#endif
