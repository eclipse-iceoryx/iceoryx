# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
# Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

if(UNIX AND NOT APPLE)
    if(CMAKE_SYSTEM_NAME MATCHES Linux)
        set(LINUX true)
    elseif(CMAKE_SYSTEM_NAME MATCHES QNX)
        set(QNX true)
    endif()
endif()

function(iox_create_asan_compile_time_blacklist BLACKLIST_FILE_PATH)
    # Suppressing Errors in Recompiled Code (Blacklist)
    # (https://clang.llvm.org/docs/AddressSanitizer.html#suppressing-errors-in-recompiled-code-blacklist)
    # More details about the syntax can be found here (https://clang.llvm.org/docs/SanitizerSpecialCaseList.html)
    if(NOT EXISTS ${BLACKLIST_FILE_PATH})
        file(WRITE  ${BLACKLIST_FILE_PATH} "# This file is auto-generated from iceoryx_platform/cmake/IceoryxPlatform.cmake\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "# src:*file_name.cpp*\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "# fun:*Test_Name*\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "# End of file\n")
    endif()
endfunction()

function(iox_create_asan_runtime_blacklist BLACKLIST_FILE_PATH)
    # Suppress errors in external libraries (https://clang.llvm.org/docs/AddressSanitizer.html#suppressing-reports-in-external-libraries)
    # List of errors generated in .inl files. These cannot be suppressed with -fsanitize-blacklist!
    # We enable sanitizer flags for core components, not in tests (mainly to avoid catching errors in test cases, at least for now)
    # NOTE : AddressSanitizer won't generate any report for the suppressed errors.
    #        Only way to see detailed errors is to disable the entries here  & run
    if(NOT EXISTS ${BLACKLIST_FILE_PATH})
        file(WRITE  ${BLACKLIST_FILE_PATH} "# This file is auto-generated from iceoryx_platform/cmake/IceoryxPlatform.cmake\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "#interceptor_via_fun:-[ClassName objCMethodToSuppress:]\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "#interceptor_via_lib:NameOfTheLibraryToSuppress\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "# End of file\n")
    endif()
endfunction()

function(iox_create_tsan_runtime_blacklist BLACKLIST_FILE_PATH)
    # (https://github.com/google/sanitizers/wiki/ThreadSanitizerSuppressions)
    # The suppression types are:
    # race              suppresses data races and use-after-free reports
    # race_top          same as race, but matched only against the top stack frame
    # thread            suppresses reports related to threads (leaks)
    # mutex             suppresses reports related to mutexes (destruction of a locked mutex)
    # signal            suppresses reports related to signal handlers (handler calls malloc())
    # deadlock          suppresses lock inversion reports
    # called_from_lib   suppresses all interceptors in a particular library
    if(NOT EXISTS ${BLACKLIST_FILE_PATH})
        file(WRITE  ${BLACKLIST_FILE_PATH} "# This file is auto-generated from iceoryx_platform/cmake/IceoryxPlatform.cmake\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "mutex:*MutexWithDeadlockDetectionsFailsWhenSameThreadTriesToUnlockItTwice*\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "mutex:*MutexWithDeadlockDetectionsFailsWhenAnotherThreadTriesToUnlock*\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "mutex:*MutexWithStallWhenLockedBehaviorDoesntUnlockMutexWhenThreadTerminates*\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "race:*\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "deadlock:*TimingTest_AttachingInCallbackWorks*\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "# End of file\n")
    endif()
endfunction()

function(iox_create_lsan_runtime_blacklist BLACKLIST_FILE_PATH)
    # Suppress known memory leaks (https://github.com/google/sanitizers/wiki/AddressSanitizerLeakSanitizer)
    # Below function/files contains memory leaks!
    # LeakSanitizer wont report the problem for the entries here , however you can find the suppression report in the log
    #
    # e.g.
    # Suppressions used:
    # count      bytes template
    #     8        642 libacl.so.1
    #     1         24 iox::UnixDomainSocket::timedReceive
    #     1         24 iox::MessageQueue::receive
    if(NOT EXISTS ${BLACKLIST_FILE_PATH})
        file(WRITE  ${BLACKLIST_FILE_PATH} "# This file is auto-generated from iceoryx_platform/cmake/IceoryxPlatform.cmake\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "#leak:libacl.so.1\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "#leak:iox::UnixDomainSocket::timedReceive\n")
        file(APPEND ${BLACKLIST_FILE_PATH} "# End of file\n")
    endif()
endfunction()

if(ADDRESS_SANITIZER OR THREAD_SANITIZER)
    if(ADDRESS_SANITIZER AND THREAD_SANITIZER)
        message( FATAL_ERROR "You can not run asan (address sanitizer) and tsan (thread sanitizer) together. Deselect one of them!" )
    endif()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(ICEORYX_SANITIZER_BLACKLIST_FILE ${CMAKE_BINARY_DIR}/sanitizer_blacklist/sanitizer_compile_time.txt)
        iox_create_asan_compile_time_blacklist(${ICEORYX_SANITIZER_BLACKLIST_FILE})

        set(ICEORYX_SANITIZER_BLACKLIST -fsanitize-blacklist=${ICEORYX_SANITIZER_BLACKLIST_FILE})

        # unset local variables , to avoid polluting global space
        unset(ICEORYX_SANITIZER_BLACKLIST_FILE )
    endif()

    iox_create_asan_runtime_blacklist(${CMAKE_BINARY_DIR}/sanitizer_blacklist/asan_runtime.txt)
    iox_create_lsan_runtime_blacklist(${CMAKE_BINARY_DIR}/sanitizer_blacklist/lsan_runtime.txt)
    iox_create_tsan_runtime_blacklist(${CMAKE_BINARY_DIR}/sanitizer_blacklist/tsan_runtime.txt)

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(ICEORYX_SANITIZER_COMMON_FLAGS -fno-omit-frame-pointer -fno-optimize-sibling-calls)

        # For using LeakSanitizer in standalone mode
        # https://github.com/google/sanitizers/wiki/AddressSanitizerLeakSanitizer#stand-alone-mode
        # Using this mode was a bit unstable
        set(ICEORYX_LEAK_SANITIZER_FLAGS -fsanitize=leak)

        set(ICEORYX_ADDRESS_SANITIZER_FLAGS -fsanitize=address -fsanitize-address-use-after-scope ${ICEORYX_SANITIZER_BLACKLIST})

        # UndefinedBehaviorSanitizer
        # -fno-sanitize-recover=... print a verbose error report and exit the program
        set(ICEORYX_UB_SANITIZER_FLAGS -fsanitize=undefined -fno-sanitize-recover=undefined)

        # ThreadSanitizer
        set(ICEORYX_THREAD_SANITIZER_FLAGS -fsanitize=thread)

        # Combine different sanitizer flags to define overall sanitization
        if(ADDRESS_SANITIZER)
            set(ICEORYX_SANITIZER_FLAGS ${ICEORYX_SANITIZER_COMMON_FLAGS} ${ICEORYX_ADDRESS_SANITIZER_FLAGS} ${ICEORYX_UB_SANITIZER_FLAGS} CACHE INTERNAL "")
        elseif(THREAD_SANITIZER)
            set(ICEORYX_SANITIZER_FLAGS ${ICEORYX_SANITIZER_COMMON_FLAGS} ${ICEORYX_THREAD_SANITIZER_FLAGS} CACHE INTERNAL "")
        endif()

        # unset local variables , to avoid polluting global space
        unset(ICEORYX_SANITIZER_BLACKLIST)
        unset(ICEORYX_SANITIZER_COMMON_FLAGS)
        unset(ICEORYX_ADDRESS_SANITIZER_FLAGS)
        unset(ICEORYX_UB_SANITIZER_FLAGS)
        unset(ICEORYX_THREAD_SANITIZER_FLAGS)
    else()
        message( FATAL_ERROR "You need to run sanitize with gcc/clang compiler." )
    endif()
endif()

if(COVERAGE)
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(ICEORYX_GRCOV_FLAGS -g -O0 -fprofile-arcs -ftest-coverage CACHE INTERNAL "")
    else(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message( FATAL_ERROR "You need to run gcov with gcc compiler." )
    endif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
endif()
