# Copyright (c) 2021 Apex.AI Inc. All rights reserved.
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

set(IOX_VERSION_STRING "2.95.6")

project(iceoryx_doc VERSION ${IOX_VERSION_STRING})

find_package(Doxygen REQUIRED)

set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_LATEX YES)
set(DOXYGEN_GENERATE_XML YES)
set(DOXYGEN_XML_OUTPUT xml)
# if some header need to be excluded from the exclude pattern have a look at INCLUDE_DIR_AND_ADDITIONAL_FILES below
set(DOXYGEN_EXCLUDE_PATTERNS "*/internal/*")
set(DOXYGEN_EXCLUDE_SYMBOLS "internal::*")

if(EXISTS "/usr/share/plantuml/plantuml.jar")
  set(DOXYGEN_PLANTUML_JAR_PATH /usr/share/plantuml/plantuml.jar)
else()
  if(DEFINED $ENV{PLANTUML_JAR_PATH})
    set(DOXYGEN_PLANTUML_JAR_PATH
        "$ENV{PLANTUML_JAR_PATH}"
        CACHE STRING "PLANTUML location.")
  else()
    message(
      WARNING
        "plantuml.jar not found at '/usr/share/plantuml/', please set environment variable PLANTUML_JAR_PATH to the correct location."
    )
  endif()
endif()

set(DOXYGEN_ALIASES
    [["concurrent=\xrefitem concurrent \"Concurrent\" \"Concurrency\" " \
"req=\xrefitem req \"Requirement\" \"Requirements\" " \
"link=\xrefitem link \"Link\" \"Links\" " \
swcomponent="@par Software Component:" \
error="@par Error Handling:" \
generatedcode="@par Generated by:"]])

# Do not add new sub-components in this list but use 'INCLUDE_DIR_AND_ADDITIONAL_FILES'
set(COMPONENTS iceoryx_platform iceoryx_hoofs iceoryx_posh iceoryx_binding_c
               iceoryx_introspection iceoryx_component)
set(COMPONENT_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_platform/generic
    ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_posh
    ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_binding_c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../tools/introspection
    ${CMAKE_CURRENT_SOURCE_DIR}/example/iceoryx_component)


list(LENGTH COMPONENTS tmp_len)
math(EXPR cmp_len "${tmp_len} - 1")

foreach(val RANGE ${cmp_len})
  list(GET COMPONENTS ${val} cmp)
  list(GET COMPONENT_DIRS ${val} cmp_dir)

  set(DOXYGEN_STRIP_FROM_INC_PATH "${cmp_dir}/include")
  set(DOXYGEN_PROJECT_NAME "${cmp}")
  message(STATUS "${cmp}  ${cmp_dir}")

  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/doc/${cmp})

  set(INCLUDE_DIR_AND_ADDITIONAL_FILES ${cmp_dir}/include)
  if(${cmp} STREQUAL iceoryx_posh)
    list(APPEND INCLUDE_DIR_AND_ADDITIONAL_FILES
      ${cmp_dir}/include/iceoryx_posh/internal/popo/base_client.hpp
      ${cmp_dir}/include/iceoryx_posh/internal/popo/base_publisher.hpp
      ${cmp_dir}/include/iceoryx_posh/internal/popo/base_server.hpp
      ${cmp_dir}/include/iceoryx_posh/internal/popo/base_subscriber.hpp
      ${cmp_dir}/include/iceoryx_posh/internal/popo/client_impl.hpp
      ${cmp_dir}/include/iceoryx_posh/internal/popo/publisher_impl.hpp
      ${cmp_dir}/include/iceoryx_posh/internal/popo/server_impl.hpp
      ${cmp_dir}/include/iceoryx_posh/internal/popo/subscriber_impl.hpp
      ${cmp_dir}/include/iceoryx_posh/internal/popo/untyped_client_impl.hpp
      ${cmp_dir}/include/iceoryx_posh/internal/popo/untyped_publisher_impl.hpp
      ${cmp_dir}/include/iceoryx_posh/internal/popo/untyped_server_impl.hpp
      ${cmp_dir}/include/iceoryx_posh/internal/popo/untyped_subscriber_impl.hpp
    )
  endif()

  if(${cmp} STREQUAL iceoryx_hoofs)
    list(APPEND INCLUDE_DIR_AND_ADDITIONAL_FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/buffer
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/cli
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/concurrent/buffer
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/concurrent/sync
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/container
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/design
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/filesystem
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/functional
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/memory
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/posix/auth
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/posix/design
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/posix/filesystem
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/posix/ipc
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/posix/sync
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/posix/time
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/posix/utility
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/posix/vocabulary
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/primitives
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/reporting
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/time
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/utility
      ${CMAKE_CURRENT_SOURCE_DIR}/../../iceoryx_hoofs/vocabulary
    )
  endif()

  doxygen_add_docs(doxygen_${cmp}
                  ${INCLUDE_DIR_AND_ADDITIONAL_FILES}
                  WORKING_DIRECTORY ${cmp_dir}/include
                  ALL)
endforeach()

# create dummy install directive to generate docu also with "make install"
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.doxygen_iceoryx_hoofs
  DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR}
)
