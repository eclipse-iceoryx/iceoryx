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
#ifndef IOX_UTILS_CXX_NEWTYPE_HPP
#define IOX_UTILS_CXX_NEWTYPE_HPP

#include "iceoryx_utils/internal/cxx/newtype/comparable.hpp"
#include "iceoryx_utils/internal/cxx/newtype/newtype_base.hpp"
#include "iceoryx_utils/internal/cxx/newtype/sortable.hpp"

namespace iox
{
namespace cxx
{
template <typename T, template <typename> class... Policies>
class NewType : public Policies<NewType<T, Policies...>>..., public newtype::NewTypeBase<T>
{
  public:
    using newtype::NewTypeBase<T>::NewTypeBase;

    using value_type = T;
};
} // namespace cxx
} // namespace iox

#endif
