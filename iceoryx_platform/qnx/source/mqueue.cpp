// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_platform/mqueue.hpp"

mqd_t iox_mq_open2(const char* name, int oflag)
{
    return mq_open(name, oflag);
}

mqd_t iox_mq_open4(const char* name, int oflag, mode_t mode, struct mq_attr* attr)
{
    return mq_open(name, oflag, mode, attr);
}
