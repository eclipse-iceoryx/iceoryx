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

#include "mqueue_mock.hpp"
#include "mocks.hpp"
#include <iostream>

std::unique_ptr<mqueue_MOCK> mqueue_MOCK::mock;
bool mqueue_MOCK::doUseMock = false;

namespace mqueue_orig
{
mqd_t (*mq_open)(const char*, int, mode_t, struct mq_attr*) =
    mocks::assignSymbol<mqd_t (*)(const char*, int, mode_t, struct mq_attr*)>("mq_open");
mqd_t (*mq_open2)(const char*, int) = mocks::assignSymbol<mqd_t (*)(const char*, int)>("mq_open");
int (*mq_unlink)(const char*) = mocks::assignSymbol<int (*)(const char*)>("mq_unlink");
int (*mq_close)(int) = mocks::assignSymbol<int (*)(int)>("mq_close");
ssize_t (*mq_receive)(int, char*, size_t, unsigned int*) =
    mocks::assignSymbol<ssize_t (*)(int, char*, size_t, unsigned int*)>("mq_receive");
ssize_t (*mq_timedreceive)(int, char*, size_t, unsigned int*, const struct timespec*) =
    mocks::assignSymbol<ssize_t (*)(int, char*, size_t, unsigned int*, const struct timespec*)>("mq_timedreceive");
int (*mq_send)(int,
               const char*,
               size_t,
               unsigned int) = mocks::assignSymbol<int (*)(int, const char*, size_t, unsigned int)>("mq_send");
int (*mq_timedsend)(int, const char*, size_t, unsigned int, const struct timespec*) =
    mocks::assignSymbol<int (*)(int, const char*, size_t, unsigned int, const struct timespec*)>("mq_timedsend");
} // namespace mqueue_orig

#if defined(QNX) || defined(QNX__) || defined(__QNX__)
int mq_unlink(const char* name)
#else
int mq_unlink(const char* name) throw()
#endif
{
    return (mqueue_MOCK::doUseMock) ? mqueue_MOCK::mock->mq_unlink(name) : mqueue_orig::mq_unlink(name);
}

mqd_t mq_open(const char* name, int oflag, mode_t mode, struct mq_attr* attr)
{
    return (mqueue_MOCK::doUseMock) ? mqueue_MOCK::mock->mq_open(name, oflag, mode, attr)
                                    : mqueue_orig::mq_open(name, oflag, mode, attr);
}

mqd_t mq_open(const char* name, int oflag)
{
    return (mqueue_MOCK::doUseMock) ? mqueue_MOCK::mock->mq_open(name, oflag) : mqueue_orig::mq_open2(name, oflag);
}

#if defined(QNX) || defined(QNX__) || defined(__QNX__)
int mq_close(int i)
#else
int mq_close(int i) throw()
#endif
{
    return (mqueue_MOCK::doUseMock) ? mqueue_MOCK::mock->mq_close(i) : mqueue_orig::mq_close(i);
}

ssize_t mq_receive(int mqdes, char* msg_ptr, size_t msg_len, unsigned int* msg_prio)
{
    return (mqueue_MOCK::doUseMock) ? mqueue_MOCK::mock->mq_receive(mqdes, msg_ptr, msg_len, msg_prio)
                                    : mqueue_orig::mq_receive(mqdes, msg_ptr, msg_len, msg_prio);
}

ssize_t
mq_timedreceive(int mqdes, char* msg_ptr, size_t msg_len, unsigned int* msg_prio, const struct timespec* abs_timeout)
{
    return (mqueue_MOCK::doUseMock) ? mqueue_MOCK::mock->mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout)
                                    : mqueue_orig::mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout);
}

int mq_send(int mqdes, const char* msg_ptr, size_t msg_len, unsigned int msg_prio)
{
    return (mqueue_MOCK::doUseMock) ? mqueue_MOCK::mock->mq_send(mqdes, msg_ptr, msg_len, msg_prio)
                                    : mqueue_orig::mq_send(mqdes, msg_ptr, msg_len, msg_prio);
}

int mq_timedsend(
    int mqdes, const char* msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec* abs_timeout)

{
    return (mqueue_MOCK::doUseMock) ? mqueue_MOCK::mock->mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout)
                                    : mqueue_orig::mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout);
}
