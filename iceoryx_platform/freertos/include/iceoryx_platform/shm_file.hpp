// Copyright (c) 2023 by NXP. All rights reserved.
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

#ifndef IOX_HOOFS_FREERTOS_SHM_FILE_HPP
#define IOX_HOOFS_FREERTOS_SHM_FILE_HPP

#include <array>
#include <cstdint>
#include <mutex>

class ShmFile
{
  public:
    static constexpr uint32_t MAX_SHM_SEGMENTS = 5U;
    static constexpr uint32_t MAX_NAME_LENGTH = 128U;

    ShmFile() noexcept;
    ShmFile(const char* name) noexcept;
    ~ShmFile();

    bool ftruncate(uint32_t size) noexcept;
    void* ptr() noexcept;

    const char* name() const noexcept;
    int fd() const noexcept;
    uint32_t size() const noexcept;
    bool empty() const noexcept;

    static std::mutex openFilesMutex;
    static std::array<ShmFile, MAX_SHM_SEGMENTS> openFiles;

  private:
    char m_name[MAX_NAME_LENGTH];
    int m_fd{0};
    uint32_t m_size{0U};
    void* m_ptr{nullptr};
    bool m_full{false};

    static int s_fdCounter;
};

#endif
