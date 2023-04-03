// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_RUNTIME_IPC_MESSAGE_HPP
#define IOX_POSH_RUNTIME_IPC_MESSAGE_HPP

#include "iox/logging.hpp"

#include <cstdint>
#include <sstream>
#include <string>

namespace iox
{
namespace runtime
{
/// @details
///    The symbol , is per default the separator.
///
///    A valid entry is an arbitrary string which does not contain the
///    separator symbol, otherwise, if it contains the separator symbol it
///    is defined as a invalid entry. A valid entry can contain none or more
///    characters.
///
///    A message is a concatination of valid entries separated by the
///    separator. A message is defined as valid if all entries contained in
///    that message are valid and it ends with the separator or it is empty,
///    otherwise it is defined as invalid.
class IpcMessage
{
  public:
    /// @brief Creates an empty and valid IPC channel message.
    IpcMessage() noexcept = default;

    /// @brief Creates a separator separated string. If one element
    ///      contains a separator symbol the IpcMessage becomes invalid
    ///      and returns false on isValid().
    /// @param[in] msg array of strings to combine in a message
    IpcMessage(const std::initializer_list<std::string>& msg) noexcept;

    /// @brief Takes a separator separated string and interprets it as
    ///      a IpcMessage. In this case the IpcMessage can only become
    ///      invalid if it is not empty and does not end with the separator.
    /// @param[in] separator separated string for a message
    IpcMessage(const std::string& msg) noexcept;

    ///  @brief Adds a new entry to the IpcMessage, if the entry is invalid
    ///          no entry is added and the IpcMessage becomes invalid.
    ///  @param[in] entry Datatype which is convertable to string via std::to_string
    template <typename T>
    IpcMessage& operator<<(const T& entry) noexcept;

    /// @brief Returns the number of entries stored in IpcMessage.
    ///          If the message is invalid the return value is undefined.
    /// @return number of entries in messaage
    uint32_t getNumberOfElements() const noexcept;

    /// @brief Returns the entry at position f_index. If f_index is larger
    ///         then the sum of the entries stored in IpcMessage it returns
    ///         std::string()
    /// @param[in] index desired entry position
    /// @return If the element exists it returns the element at f_index
    ///         otherwise an empty string
    //          If the message is invalid the return value is undefined.
    std::string getElementAtIndex(const uint32_t index) const noexcept;

    /// @brief returns if an entry is valid.
    ///      Non valid entries are containing at least one separator
    /// @param[in] entry sstring to check
    /// @return
    ///      true = if it is a valid entry otherwise false
    bool isValidEntry(const std::string& entry) const noexcept;

    /// @brief check if the message is valid
    /// @return  If one element in the CTor initializer_list was invalid it returns false, otherwise true.
    bool isValid() const noexcept;

    /// @brief The message is casted to the actual separator separated
    ///         string
    ///        If the message is invalid the return value is undefined.
    /// @return the current message as separator separated string
    std::string getMessage() const noexcept;

    /// @brief Takes a separator separated string and interprets it as
    ///      a IpcMessage. In this case the IpcMessage can only become
    ///      invalid if it is not empty and does not end with the separator.
    ///      All the entries that were stored previously in the IpcMessage
    ///      will be cleared after a call to setMessage.
    /// @param[in] separator separated string for the message
    void setMessage(const std::string& msg) noexcept;

    /// @brief Clears the message. After a call to clearMessage() the
    //      message becomes valid again.
    void clearMessage() noexcept;

    /// @brief Adds a new entry to the IpcMessage, if the entry is invalid
    ///         no entry is added and the IpcMessage becomes invalid.
    /// @tparam Datatype which is convertable to string via
    /// std::stringstream
    /// @param[in] entry to add to the message
    template <typename T>
    void addEntry(const T& entry) noexcept;

    /// @brief Compares two IpcMessages to be equal
    /// @param rhs IpcMessage to compare with
    bool operator==(const IpcMessage& rhs) const noexcept;

  private:
    static const char m_separator; // default value is ,
    std::string m_msg;
    bool m_isValid{true};
    uint32_t m_numberOfElements{0};
};

} // namespace runtime
} // namespace iox


#include "iceoryx_posh/internal/runtime/ipc_message.inl"

#endif // IOX_POSH_RUNTIME_IPC_MESSAGE_HPP
