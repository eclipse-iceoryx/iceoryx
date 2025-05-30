# Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
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

cmake_minimum_required(VERSION 3.16)
set(test_iceoryx_hoofs_VERSION 0)
project(test_iceoryx_hoofs VERSION ${test_iceoryx_hoofs_VERSION})

find_package(Threads REQUIRED)
find_package(iceoryx_hoofs_testing REQUIRED)

set(PROJECT_PREFIX "hoofs")

set(MODULETESTS_SRC "")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/main_test_hoofs_modules.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/error_reporting/test_auxiliary_types.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/error_reporting/test_custom_error_reporting.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/error_reporting/test_default_error_handler.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/error_reporting/test_error_kind.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/error_reporting/test_error_reporting_macros.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/error_reporting/test_error_reporting_violation.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/error_reporting/test_testing_error_handler.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_buffer_stack.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_cli_cli_definition.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_cli_command_line_parser.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_cli_option.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_cli_option_definition.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_concurrent_mpmc_index_queue.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_concurrent_mpmc_lockfree_queue.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_concurrent_mpmc_lockfree_queue_cyclic_index.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_concurrent_mpmc_resizeable_lockfree_queue.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_concurrent_spsc_fifo.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_concurrent_spsc_sofi.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_container_fixed_position_container.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_container_forward_list.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_container_list.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_container_uninitialized_array.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_container_vector.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_design_functional_interface_and_then.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_design_functional_interface_common.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_design_functional_interface_concat_multiple_calls.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_design_functional_interface_expect.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_design_functional_interface_or_else.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_design_functional_interface_types.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_design_functional_interface_value_or.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_design_move_and_copy_helper.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_design_newtype.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_design_polymorphic_handler.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_design_static_lifetime_guard.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_functional_function.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_functional_function_ref.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_memory_bump_allocator.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_memory_memory.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_memory_relative_pointer.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_memory_relative_pointer_data.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_memory_relocatable_ptr.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_memory_scope_guard.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_memory_static_storage.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_memory_unique_ptr.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_access_rights.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_path_and_file_verifier.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_posix_call.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_primitives_algorithm.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_primitives_size.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_primitives_type_traits.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_reporting_console_logger.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_reporting_logging.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_reporting_logstream_arithmetic.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_reporting_logstream.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_reporting_logstream_hex_oct_bin.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_time_unit_duration.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_utility_convert.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_utility_deprecation_marker.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_utility_into.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_utility_serialization.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_utility_std_chrono_support.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_utility_std_string_support.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_utility_unique_id.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_vocabulary_expected.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_vocabulary_optional.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_vocabulary_semantic_string.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_vocabulary_span.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_vocabulary_string_comparison.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_vocabulary_string_concatenate_append_and_insert.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_vocabulary_string.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_vocabulary_string_ctor_and_assign.cpp")
list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_vocabulary_variant.cpp")
if(NOT IOX_USE_HOOFS_SUBSET_ONLY)
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_concurrent_mpmc_loffli.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_concurrent_periodic_task.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_concurrent_smart_lock.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_filesystem_file_reader.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_filesystem_filesystem.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_file.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_file_lock.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_filesystem_posix_acl.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_ipc_unix_domain_sockets.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_named_semaphore.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_mutex.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_semaphore_interface.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_shared_memory.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_shared_memory_object.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_signal_handler.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_sync_signal_watcher.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_thread.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_posix_unnamed_semaphore.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_time_adaptive_wait.cpp")
    list(APPEND MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/test_time_deadline_timer.cpp")
endif()

file(GLOB_RECURSE INTEGRATIONTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/integrationtests/*.cpp")
file(GLOB_RECURSE MOCKTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/mocktests/*.cpp")

set(STRESSTESTS_SRC "")
list(APPEND STRESSTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/stresstests/test_hoofs_stress.cpp")
list(APPEND STRESSTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/stresstests/test_mpmc_lockfree_queue_stresstest.cpp")
if(NOT IOX_USE_HOOFS_SUBSET_ONLY)
    list(APPEND STRESSTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/stresstests/test_mpmc_resizeable_lockfree_queue_stresstest.cpp")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_PREFIX}/test)

set(TEST_LINK_LIBS
    ${CODE_COVERAGE_LIBS}
    GTest::gtest
    GTest::gmock
    iceoryx_hoofs::iceoryx_hoofs
    iceoryx_hoofs_testing::iceoryx_hoofs_testing
)

iox_add_executable( TARGET                  ${PROJECT_PREFIX}_moduletests
                    INCLUDE_DIRECTORIES     .
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              dl
                    FILES                   ${MODULETESTS_SRC}
)

iox_add_executable( TARGET                  ${PROJECT_PREFIX}_mocktests
                    INCLUDE_DIRECTORIES     .
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              dl
                    FILES                   ${MOCKTESTS_SRC}
)

iox_add_executable( TARGET                  ${PROJECT_PREFIX}_integrationtests
                    INCLUDE_DIRECTORIES     .
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              dl
                    FILES                   ${INTEGRATIONTESTS_SRC}
)

iox_add_executable( TARGET                  ${PROJECT_PREFIX}_stresstests
                    INCLUDE_DIRECTORIES     .
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              dl
                    FILES                   ${STRESSTESTS_SRC}
)

iox_add_executable( TARGET                  test_stress_spsc_sofi
                    INCLUDE_DIRECTORIES     .
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              dl
                    FILES                   stresstests/sofi/test_stress_spsc_sofi.cpp
)

add_subdirectory(stresstests/benchmark_optional_and_expected)

target_compile_options(${PROJECT_PREFIX}_moduletests PRIVATE ${ICEORYX_TEST_CXX_FLAGS})
target_compile_options(${PROJECT_PREFIX}_mocktests PRIVATE ${ICEORYX_TEST_CXX_FLAGS})
target_compile_options(${PROJECT_PREFIX}_integrationtests PRIVATE ${ICEORYX_TEST_CXX_FLAGS})
target_compile_options(test_stress_spsc_sofi PRIVATE ${ICEORYX_TEST_CXX_FLAGS})
