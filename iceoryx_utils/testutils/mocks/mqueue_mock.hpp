// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#ifndef MQUEUE_MOCK_HPP_INCLUDED
#define MQUEUE_MOCK_HPP_INCLUDED

#include "test.hpp"
#include <mqueue.h>
#include <mutex>

using namespace ::testing;
using ::testing::Return;

class mqueue_MOCK
{
  public:
    mqueue_MOCK() = default;
    virtual ~mqueue_MOCK() = default;

    MOCK_METHOD4(mq_open, mqd_t(const char*, int, mode_t, struct mq_attr*));
    MOCK_METHOD2(mq_open, mqd_t(const char*, int));
    MOCK_METHOD1(mq_unlink, int(const char*));
    MOCK_METHOD1(mq_close, int(int));
    MOCK_METHOD4(mq_receive, ssize_t(int, char*, size_t, unsigned int*));
    MOCK_METHOD5(mq_timedreceive, ssize_t(int, char*, size_t, unsigned int*, const struct timespec*));
    MOCK_METHOD4(mq_send, int(int, const char*, size_t, unsigned int));
    MOCK_METHOD5(mq_timedsend, int(int, const char*, size_t, unsigned int, const struct timespec*));

    static bool doUseMock; // = true
    static std::unique_ptr<mqueue_MOCK> mock;
};
#if defined(QNX) || defined(QNX__) || defined(__QNX__)
int mq_unlink(const char* name);
#else
int mq_unlink(const char* name) throw();
#endif

mqd_t mq_open(const char* name, int oflag, mode_t mode, struct mq_attr* attr);
mqd_t mq_open(const char* name, int oflag);

#if defined(QNX) || defined(QNX__) || defined(__QNX__)
int mq_close(int i);
#else
int mq_close(int i) throw();
#endif

ssize_t mq_receive(int mqdes, char* msg_ptr, size_t msg_len, unsigned int* msg_prio);
ssize_t
mq_timedreceive(int mqdes, char* msg_ptr, size_t msg_len, unsigned int* msg_prio, const struct timespec* abs_timeout);
int mq_send(int mqdes, const char* msg_ptr, size_t msg_len, unsigned int msg_prio);
int mq_timedsend(
    int mqdes, const char* msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec* abs_timeout);

#endif
