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
#ifndef IOX_UTILS_ERROR_HANDLING_ERROR_HANDLING_HPP
#define IOX_UTILS_ERROR_HANDLING_ERROR_HANDLING_HPP

#include "iceoryx_utils/cxx/generic_raii.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

#include <assert.h>
#include <functional>
#include <iostream>
#include <mutex>

namespace iox
{
// clang-format off
#define ICEORYX_ERRORS(error) \
    error(NO_ERROR)\
    error(FILEREADER__FAILED_TO_OPEN_FILE) \
    error(POSH__RUNTIME_IS_CREATED_MULTIPLE_TIMES) \
    error(POSH__RUNTIME_SENDERPORT_NOT_UNIQUE) /* @deprecated #25 */ \
    error(POSH__RUNTIME_ROUDI_SENDERLIST_FULL) /* @deprecated #25 */ \
    error(POSH__RUNTIME_SENDERPORT_CREATION_UNDEFINED_BEHAVIOR) /* @deprecated #25 */ \
    error(POSH__RUNTIME_WRONG_MESSAGE_QUEUE_RESPONSE) /* @deprecated #25 */ \
    error(POSH__RUNTIME_PUBLISHER_PORT_NOT_UNIQUE) \
    error(POSH__RUNTIME_PUBLISHER_PORT_CREATION_UNDEFINED_BEHAVIOR) \
    error(POSH__RUNTIME_SUBSCRIBER_PORT_CREATION_UNDEFINED_BEHAVIOR) \
    error(POSH__RUNTIME_ROUDI_PUBLISHER_LIST_FULL) \
    error(POSH__RUNTIME_ROUDI_SUBSCRIBER_LIST_FULL) \
    error(POSH__RUNTIME_ROUDI_CONDITION_VARIABLE_LIST_FULL) \
    error(POSH__RUNTIME_ROUDI_REQUEST_PUBLISHER_WRONG_MESSAGE_QUEUE_RESPONSE) \
    error(POSH__RUNTIME_ROUDI_REQUEST_SUBSCRIBER_WRONG_MESSAGE_QUEUE_RESPONSE) \
    error(POSH__RUNTIME_ROUDI_REQUEST_CONDITION_VARIABLE_WRONG_MESSAGE_QUEUE_RESPONSE) \
    error(POSH__RUNTIME_ROUDI_GET_MW_INTERFACE_WRONG_MESSAGE_QUEUE_RESPONSE) \
    error(POSH__RUNTIME_ROUDI_CREATE_RUNNABLE_WRONG_MESSAGE_QUEUE_RESPONSE) \
    error(POSH__RUNTIME_ROUDI_GET_MW_APPLICATION_WRONG_MESSAGE_QUEUE_RESPONSE) \
    error(POSH__RUNTIME_ROUDI_CONDITION_VARIABLE_CREATION_UNDEFINED_BEHAVIOR) \
    error(POSH__PORT_MANAGER_SENDERPORT_NOT_UNIQUE) \
    error(POSH__MEMPOOL_POSSIBLE_DOUBLE_FREE) \
    error(POSH__RECEIVERPORT_DELIVERYFIFO_OVERFLOW) \
    error(POSH__SENDERPORT_SAMPLE_SIZE_CHANGED_FOR_ACTIVE_PORT) \
    error(POSH__SENDERPORT_ACTIVATE_FIELD_WITHOUT_DATA) \
    error(POSH__SENDERPORT_FIELD_SUBSCRIBE_WITHOUT_DATA) \
    error(POSH__SENDERPORT_ACTIVATE_FIELD_WHEN_LATCHED_TOPIC_ACTIVATED) \
    error(POSH__SENDERPORT_ALLOCATE_FAILED) \
    error(POSH__SENDERPORT_SUBSCRIBER_LIST_OVERFLOW) \
    error(POSH__SHM_APP_BASEADDRESS_VIOLATES_SPECIFICATION) \
    error(POSH__SHM_APP_SEGMENT_BASEADDRESS_VIOLATES_SPECIFICATION) \
    error(POSH__SHM_APP_MAPP_ERR) \
    error(POSH__SHM_APP_SEGMENT_MAPP_ERR) \
    error(POSH__SHM_APP_SEGMENT_COUNT_OVERFLOW) \
    error(POSH__INTERFACEPORT_CAPRO_MESSAGE_DISMISSED) \
    error(POSH__SERVICE_DISCOVERY_INSTANCE_CONTAINER_OVERFLOW) \
    error(POSH__SERVICE_DISCOVERY_FIND_SERVICE_CALLBACKS_CONTAINER_OVERFLOW) \
    error(POPO__APPLICATION_PORT_QUEUE_OVERFLOW) \
    error(POPO__CHUNK_DISTRIBUTOR_OVERFLOW_OF_QUEUE_CONTAINER) \
    error(POPO__CHUNK_DISTRIBUTOR_CLEANUP_DEADLOCK_BECAUSE_BAD_APPLICATION_TERMINATION) \
    error(POPO__CHUNK_SENDER_INVALID_CHUNK_TO_FREE_FROM_USER) \
    error(POPO__CHUNK_SENDER_INVALID_CHUNK_TO_SEND_FROM_USER) \
    error(POPO__CHUNK_RECEIVER_INVALID_CHUNK_TO_RELEASE_FROM_USER) \
    error(POPO__CHUNK_LOCKING_ERROR) \
    error(POPO__CHUNK_UNLOCKING_ERROR) \
    error(POPO__CAPRO_PROTOCOL_ERROR) \
    error(POPO__CONDITION_VARIABLE_DATA_FAILED_TO_CREATE_SEMAPHORE) \
    error(POPO__TYPED_UNIQUE_ID_ROUDI_HAS_NO_DEFINED_UNIQUE_ID) \
    error(POPO__TYPED_UNIQUE_ID_ROUDI_HAS_ALREADY_DEFINED_UNIQUE_ID) \
    error(POPO__TYPED_UNIQUE_ID_OVERFLOW) \
    error(POPO__WAITSET_CONDITION_LIFETIME_ISSUE) \
    error(POPO__WAITSET_COULD_NOT_DETACH_CONDITION) \
    error(POPO__WAITSET_CONDITION_VECTOR_OVERFLOW) \
    error(MEPOO__MEMPOOL_CONFIG_MUST_BE_ORDERED_BY_INCREASING_SIZE) \
    error(MEPOO__MEMPOOL_GETCHUNK_CHUNK_IS_TOO_LARGE) \
    error(MEPOO__MEMPOOL_CHUNKSIZE_MUST_BE_LARGER_THAN_SHARED_MEMORY_ALIGNMENT_AND_MULTIPLE_OF_ALIGNMENT) \
    error(MEPOO__MEMPOOL_ADDMEMPOOL_AFTER_GENERATECHUNKMANAGEMENTPOOL) \
    error(MEPOO__TYPED_MEMPOOL_HAS_INCONSISTENT_STATE) \
    error(MEPOO__TYPED_MEMPOOL_MANAGEMENT_SEGMENT_IS_BROKEN) \
    error(MEPOO__SEGMENT_CONTAINER_OVERFLOW) \
    error(MEPOO__USER_WITH_MORE_THAN_ONE_WRITE_SEGMENT) \
    error(MEPOO__SEGMENT_COULD_NOT_APPLY_POSIX_RIGHTS_TO_SHARED_MEMORY) \
    error(MEPOO__SEGMENT_UNABLE_TO_CREATE_SHARED_MEMORY_OBJECT) \
    error(PORT_POOL__SENDERLIST_OVERFLOW) /* @deprecated #25 */ \
    error(PORT_POOL__RECEIVERLIST_OVERFLOW) /* @deprecated #25 */ \
    error(PORT_POOL__PUBLISHERLIST_OVERFLOW) \
    error(PORT_POOL__SUBSCRIBERLIST_OVERFLOW) \
    error(PORT_POOL__INTERFACELIST_OVERFLOW) \
    error(PORT_POOL__APPLICATIONLIST_OVERFLOW) \
    error(PORT_POOL__RUNNABLELIST_OVERFLOW) \
    error(PORT_POOL__CONDITION_VARIABLE_LIST_OVERFLOW) \
    error(PORT_MANAGER__PORT_POOL_UNAVAILABLE) \
    error(PORT_MANAGER__INTROSPECTION_MEMORY_MANAGER_UNAVAILABLE) \
    error(PORT_MANAGER__HANDLE_PUBLISHER_PORTS_INVALID_CAPRO_MESSAGE) \
    error(PORT_MANAGER__HANDLE_SUBSCRIBER_PORTS_INVALID_CAPRO_MESSAGE) \
    error(ROUDI_COMPONENTS__SHARED_MEMORY_UNAVAILABLE) \
    error(MQ_UNKNOWN_MSG) \
    error(MQ_INVALID_MSG) \
    error(MQ_INTERFACE__REG_ROUDI_NOT_AVAILABLE) \
    error(MQ_INTERFACE__REG_UNABLE_TO_WRITE_TO_ROUDI_MQ) \
    error(MQ_INTERFACE__REG_ACK_INVALIG_NUMBER_OF_PARAMS) \
    error(MQ_INTERFACE__REG_ACK_NO_RESPONSE) \
    error(MQ_INTERFACE__CHECK_MQ_MAPS_TO_FILE) \
    error(POSIX_WRAPPER__FAILED_TO_CREATE_SEMAPHORE) \
    error(POSIX_TIMER__FIRED_TIMER_BUT_STATE_IS_INVALID) \
    error(POSIX_TIMER__TIMERPOOL_OVERFLOW) \
    error(POSIX_TIMER__INCONSISTENT_STATE) \
    error(POSIX_TIMER__CALLBACK_RUNTIME_EXCEEDS_RETRIGGER_TIME) \
    error(VARIANT_QUEUE__UNSUPPORTED_QUEUE_TYPE)

// clang-format on

// DO NOT TOUCH START, you can doodle around with the lines above!!!
#define CREATE_ICEORYX_ERROR_ENUM(name) k##name,
#define CREATE_ICEORYX_ERROR_STRING(name) #name,

enum class Error : uint32_t
{
    ICEORYX_ERRORS(CREATE_ICEORYX_ERROR_ENUM)
};

/// @brief the available error levels
/// FATAL
/// - Log message with FATAL
/// - RouDi cannot recover from that error. RouDi is terminated
/// - Assert (in DEBUG) and terminate
/// - Reporting code must handle this and continue or go to a save state. Error handler could return (e.g. in test)
/// SEVERE
/// - Log message with ERROR
/// - RouDi can still run. Error is reported
/// - Assert in DEBUG, in RELEASE continue to run
/// - Reporting code must handle this and continue
/// MODERATE
/// - Log message with ERROR
/// - RouDi can still run. Error is reported
/// - NO assert
/// - Reporting code must handle this and continue
enum class ErrorLevel : uint32_t
{
    /// Log error entry + Assert + terminate
    FATAL,
    /// warning log entry + Assert
    SEVERE,
    /// warning log entry
    MODERATE
};

using HandlerFunction = std::function<void(const Error error, const std::function<void()>, const ErrorLevel)>;

/// @brief This handler is needed for unit testing, special debugging cases and
///         other corner cases where we'd like to explicitly suppress the
///         error handling.
class ErrorHandler
{
    friend void errorHandler(const Error error, const std::function<void()> errorCallBack, const ErrorLevel level);

  public:
    static cxx::GenericRAII SetTemporaryErrorHandler(const HandlerFunction& newHandler);

    static const char* ToString(const Error error);

  protected:
    static void ReactOnErrorLevel(const ErrorLevel level, const char* errorText);

  private:
    static void DefaultHandler(const Error error,
                               const std::function<void()> errorCallBack,
                               const ErrorLevel level = ErrorLevel::FATAL);

    static const char* errorNames[];
    static iox::HandlerFunction handler;
    /// Needed, if you want to exchange the handler. Remember the old one and call it if it is not your error. The error
    /// mock needs to be the last one exchanging the handler in tests.
    static std::mutex handler_mutex;
};

/// @brief Howto use the error handler correctly
///     1.) If the error you would like to handle is not listed in ICEORYX_ERRORS(error)\...
///             macro just add them like:
///             error(MODULE_NAME__MY_FUNKY_ERROR)
///         Attention: Create an error after the following convention:
///             MODULE_NAME__A_CLEAR_BUT_SHORT_ERROR_DESCRIPTION
///         And a long name is alright!
///
///     2.) Call errorHandler(Error::kMODULE_NAME__MY_FUNKY_ERROR);
///             Please pay attention to the "k" prefix
///         The defaults for errorCallback and ErrorLevel can also be overwritten:
///             errorHandler(
///                 Error::kMODULE_NAME__MY_FUNKY_ERROR,
///                 []{ std::cout << "MyCustomCallback" << std::endl; },
///                 ErrorLevel::MODERATE
///             );
///
/// @code
/// class PrettyClass {
///     float division(float a, float b) {
///         if ( b == 0.0f ) {
///             errorHandler(Error::kPRETTY_CLASS__DIVISION_BY_ZERO);
///         }
///     }
/// };
/// @endcode
///
/// @code
/// bool called = false;
/// auto temporaryErrorHandler = ErrorHandler::SetTemporaryErrorHandler(
///     [&](const Error e, std::function<void()>, const ErrorLevel) {
///         called = true;
///     });
///
/// errorHandler(Error::kTEST__ASSERT_CALLED);
/// ASSERT_TRUE(called);
/// @endcode
void errorHandler(const Error error,
                  const std::function<void()> errorCallBack = std::function<void()>(),
                  const ErrorLevel level = ErrorLevel::FATAL);
} // namespace iox

#endif // IOX_UTILS_ERROR_HANDLING_ERROR_HANDLING_HPP
