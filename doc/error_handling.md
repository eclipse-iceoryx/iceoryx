# Contents
1. [Logging](#Logging)
2. [Error Handling](#Error-Handling)
3. [Usage](#Usage)
3. [Open Points](#Open-Points)


# Logging
Logging is performed by the iceoryx internal logger. The logger API implements a subset of the Autosar log and trace interface ara::log.
The logger is used internally for error logging but can be used by the user as well in application code.

## Logger
The logger is responsible for logging information about the state of the system to a configurable stream (includes files). 
In the future it may be extended to include logging over a network to a remote server or similar. 

The logger is thread-safe and can hence be safely used from multiple threads concurrently.
Currently the logger is synchronous but may support asynchronous logging in the future.

## Log Levels

The following log levels are supported, ordered by criticality from lowest to highest.

* VERBOSE - all available information is printed
* DEBUG - information to support debugging on developer side
* INFO - run state information for the user
* WARN - indicates a potential problem which requires investigation
* ERR - an error occured that may be handled on application side or by RouDi
* FATAL - an error occured and RouDi is unable to continue

For ERR and FATAL see also error levels MODERATE, SEVERE (logged with LogErr) and FATAL (logged with LogFatal) in [Error Levels](##Error-Levels).


# Error Handling
Error handling is performed by the error handler which handles errors occuring in the subcomponents of iceoryx::posh.

## Error Handler
The error handler is called internally when an error is detected in the iceoryx middleware dameon (RouDi) or the iceoryx runtime. The error handler should only be called in exceptional situations (invalid access errors, out of resources etc.) and not in circumstances that occur regularly (it is sort of an exception replacement).

If the exceptional situation can be resolved without calling the error handler (e.g. by delegating an appropriate return value to the caller), this should be preferred (since the error handler is a last resort mechanism).

It is not supposed to be called by applications at any time.

## Technical Requirements
* The error handler must be reentrant.
* The error handler must be thread-safe.
* The error handler uses the logger but the logger cannot depend on the error handler.
* When a fatal error is detected and termination is required, the reporting thread shuts down the RouDi gracefully.
* A custom error handler function can be installed (e.g. for testing).
* When the reaction on an error is just logging, computation in other threads shall not be influenced.
* When the error does not require termination, the error handler must return eventually.

## Error Levels
The following error levels are supported.

* MODERATE 
* SEVERE 
* FATAL 

### MODERATE
A recoverable error. Leads to a error log entry (LogErr) and continues execution. In the future a customizable configuartion is supposed to decide whether and how to continue, but this option is not fully integrated yet.

**Example:**
1) Roudi receives an unexpected message and discards it. The remaining communication proceeds normally.
2) A port requested by an application cannot be provided due to e.g. resource exhaustion.

### SEVERE
RouDi may continue but applications may be compromised or the functionality reduced. Leads to a error log entry (LogErr) and assert, terminating execution in Debug Mode. The handler must return to be able to continue execution in Release Mode. In the application continue according to a customizable configuration.

**Example:**
A message queue is overflowing and messages are lost. RouDi can continue but lost data may affect applications.


### FATAL
RouDi cannot continue and will shut down. Leads to a error log entry (LogErr), assert and std::terminate, terminating execution in Debug and Release Mode. 
Before calling terminate, a 3rd party error is informed (if configured).
The handler is not required to return here (since this may not be always possible or reasonable). The reporting code should still try to proceed to a safe state if possible in order to improve testability in case of such errors.

A fatal error in the runtime terminates the application.

**Example:**
RouDi is unable to allocate sufficient shared memory.

## Error Codes and Additional Information

Currently error codes are used to identify the location of an error. These are provided as an enum in *error_handling.hpp*. To allow a mapping to error location, these have to be different for each error.

Note that this may not be the long term solution, as file, line and function information may be added (using \_\_FILE\_\_, \_\_LINE\_\_ and \_\_func\_\_). This would require using macros for the error handler call in a future implementation.

In addition a user callback may be provided. It cannot take arguments at the moment but this may also be extended.


## Expects and Ensures

These assert-like constructs are used to document assumptions in the code which are checked (at least) in Debug Mode. It should be possible to leave them active in Release Mode as well if desired. If the condition is violated they print the condition, the location of occurence in the code and terminate the program execution.

Since they are not necessarily active in Release Mode, they cannot be used to handle errors (currently they are, but this might change in the future). Their purpose is to detect misuse or bugs of the API early in Debug Mode or to verify a result of an algorithm before returning. In this way, assumptions of the developer are made explicit without causing overhead when not needed. Therefore errors to be caught by Expected and Ensures are considered bugs and need to be fixed or the underlying assumptions and algorithms changed. This is in contrast to errors which are expected to occur during runtime which are handled by the error handler (i.e. a system resource cannot be obtained).

Although Expects end Ensures behave the same, the former is used to signify a precondition (e.g. arguments of a function) is checked, while the latter indicates a postcondition check (e.g. result of a function before returning)

Examples include expecting pointers that are not null (as input, intermediate or final result) or range checks of variables.

## cxx::expected

**cxx::expected<T, E>** is a template which either holds the result of the computation of type **T** or an object of error type **E**. The latter can be used to obtain additional information about the error, e.g. an error code.
In a way this extends error codes and may act as kind of an replacement of exceptions. It is usually used as return type of functions which may fail for various reasons and should be used if the error is supposed to be delegated to the caller and handled in the caller context.
It is also possible to further propagate the error to the next function in the call-stack (since this must be done explicitly, this is comparable to rethrowing an exception).

It is possible to use the *nodiscard* option to force the user to handle the returned cxx::expected.
If the error cannot be handled at a higher level by the caller, the error handler needs to be used.

Examples include wrapping third party API functions that return error codes or obtaining a value from a container when this can fail for some reason (e.g. container is empty). If no additional information about the error is available or required, **cxx::optional<T>** can be used instead.

It may be worth to consider renaming cxx::expected to cxx::result in the future, which is more in line with languages such as *Rust* and conveys the meaning more clearly.

## Error Handling in posh
Error logging shall be done by the logger only, no calls to std::cerr or similar should be performed.

All the methods presented (cxx::expected, Expects and Ensures and the error handler) can be used in posh. The appropriate way depends on the type of error scenario (cf. the respective sections for examples). The error handler should be considered the last option.

## Error Handling in utils
Error logging is currently done by calls to cerr. In the future those might be redirected to the logger.

The error handler cannot be used in utils. 

Whether it is appropriate to use std::expected even if STL compatility is broken by doing so depends on the circumstances and needs to be discussed on a case-by-case basis. If the function has no STL counterpart std::expected can be used freely to communicate potential failure to the caller.

It should be noted that since currently Expects and Ensures are active at release mode, prolific usage of these will incur a runtime cost. Since this is likely to change in the future, it is still advised to use them to document the developers intentions.

## Interface for 3rd Party Code

Error handler as well as logger shall be able to use or redirect to 3rd party error handling or logging libraries in the future. Currently this is not supported.

# Usage

## Logger


## Error Handler


## Expects and Ensures

## cxx::expected
 


# Open Points

3rd party error handling
centralized error handling
Overriding specific error reaction based on error codes
Backtrace, distinguih between debug and release (or release build with extended information usable for debug purposes)

Handle errors in runtime in a configurable way (hooks)

use of expected/optional

return in case of fatal error

debug/release mode

assert

## Future Requirements

LogErr without error handler?