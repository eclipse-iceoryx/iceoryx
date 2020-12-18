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

#if !defined(_WIN32) && !defined(__APPLE__)

#include "mqueue_mock.hpp"
#include "mocks.hpp"
#include <iostream>

std::unique_ptr<mqueue_MOCK> mqueue_MOCK::mock;
bool mqueue_MOCK::doUseMock = false;

#if defined(QNX) || defined(QNX__) || defined(__QNX__)
int mq_unlink(const char* name)
#else
int mq_unlink(const char* name) throw()
#endif
{
    return (mqueue_MOCK::doUseMock) ? mqueue_MOCK::mock->mq_unlink(name)
                                    : STATIC_FUNCTION_LOADER_AUTO_DEDUCE(mq_unlink)(name);
}

mqd_t mq_open(const char* name, int oflag, mode_t mode, struct mq_attr* attr)
{
    return (mqueue_MOCK::doUseMock)
               ? mqueue_MOCK::mock->mq_open(name, oflag, mode, attr)
               : STATIC_FUNCTION_LOADER_MANUAL_DEDUCE(mqd_t(*)(const char*, int, mode_t, struct mq_attr*),
                                                      mq_open)(name, oflag, mode, attr);
}

mqd_t mq_open(const char* name, int oflag)
{
    return (mqueue_MOCK::doUseMock)
               ? mqueue_MOCK::mock->mq_open(name, oflag)
               : STATIC_FUNCTION_LOADER_MANUAL_DEDUCE(mqd_t(*)(const char*, int), mq_open2)(name, oflag);
}

#if defined(QNX) || defined(QNX__) || defined(__QNX__)
int mq_close(int i)
#else
int mq_close(int i) throw()
#endif
{
    return (mqueue_MOCK::doUseMock) ? mqueue_MOCK::mock->mq_close(i) : STATIC_FUNCTION_LOADER_AUTO_DEDUCE(mq_close)(i);
}

ssize_t mq_receive(int mqdes, char* msg_ptr, size_t msg_len, unsigned int* msg_prio)
{
    return (mqueue_MOCK::doUseMock) ? mqueue_MOCK::mock->mq_receive(mqdes, msg_ptr, msg_len, msg_prio)
                                    : STATIC_FUNCTION_LOADER_AUTO_DEDUCE(mq_receive)(mqdes, msg_ptr, msg_len, msg_prio);
}

ssize_t
mq_timedreceive(int mqdes, char* msg_ptr, size_t msg_len, unsigned int* msg_prio, const struct timespec* abs_timeout)
{
    return (mqueue_MOCK::doUseMock)
               ? mqueue_MOCK::mock->mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout)
               : STATIC_FUNCTION_LOADER_AUTO_DEDUCE(mq_timedreceive)(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout);
}

int mq_send(int mqdes, const char* msg_ptr, size_t msg_len, unsigned int msg_prio)
{
    return (mqueue_MOCK::doUseMock) ? mqueue_MOCK::mock->mq_send(mqdes, msg_ptr, msg_len, msg_prio)
                                    : STATIC_FUNCTION_LOADER_AUTO_DEDUCE(mq_send)(mqdes, msg_ptr, msg_len, msg_prio);
}

int mq_timedsend(
    int mqdes, const char* msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec* abs_timeout)

{
    return (mqueue_MOCK::doUseMock)
               ? mqueue_MOCK::mock->mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout)
               : STATIC_FUNCTION_LOADER_AUTO_DEDUCE(mq_timedsend)(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout);
}
#endif
