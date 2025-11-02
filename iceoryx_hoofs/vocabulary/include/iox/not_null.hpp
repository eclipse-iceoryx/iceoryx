// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT
#ifndef IOX_HOOFS_VOCABULARY_NOT_NULL_HPP
#define IOX_HOOFS_VOCABULARY_NOT_NULL_HPP

#include "iox/assertions.hpp"
#include "iox/type_traits.hpp"

namespace iox
{
template <typename T, typename = typename std::enable_if<std::is_pointer<T>::value, void>::type>
struct not_null
{
  public:
    // this class should behave like a pointer which never can be nullptr, adding explicit
    // would defeat the purpose
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    not_null(T t) noexcept
        : m_value(t)
    {
        IOX_ENFORCE(t != nullptr, "Parameter must not be a 'nullptr'");
    }

    // AXIVION Next Construct AutosarC++19_03-A13.5.2,AutosarC++19_03-A13.5.3:this should behave like a pointer which never can be nullptr,
    // adding explicit would defeat the purpose
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr operator T() const noexcept
    {
        return m_value;
    }

  private:
    T m_value;
};
} // namespace iox
#endif // IOX_HOOFS_VOCABULARY_NOT_NULL_HPP
