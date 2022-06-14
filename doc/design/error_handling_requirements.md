# Error Handling

## Faults, Errors and Failures

We use the following error terminology.

1. A **fault** is a design flaw, a programming error or can e.g. be caused by unforseen use
   (oversight of a use case is basically also a design flaw).
2. An **error** is the result of a fault and manifests as an incorrect or inconsistent (intermediate) result.
   An error may not be externally observable by the user of the system.
3. A **failure** occurs if the system observably acts in contradiction to its specification.

This reasonably assumes that errors are always caused by faults. If these errors are not detected
they can cause failures. By detecting and handling errors failures are avoided.

The goal of this document is to define error handling requirements.

## Error Detection vs. Error Handling

To handle an error it obviously needs to be detected first. To do so, the code must contain conditionals that check
for inconsistent states, unavailability of resources, return codes of system calls etc.
This means that the error has to be anticipated by the system designer,
i.e. we cannot handle unforseen errors since they are not detected by definition.

Error handling is the execution of handling code once the error was detected.
This ranges from simple logging to sophisticated recovery or shutdown code and depends on the error in question.
We distinguish between recoverable and non-recoverable error handling.

### Recoverable Error Handling

A recoverable error handling strategy allows the system to continue operation after the handling code
was executed successfully. Exceptions are an example of such a mechanism but also any handler code
that does not result in termination. If the handling code itself fails (e.g. insufficient resources)
no recovery is possible and the system terminates.

### Non-recoverable Error Handling

A non-recoverable error handling strategy will always call terminate after the handling code was executed.
Handler code may be used to initiate a graceful shutdown. An example are (strict) asserts.

## Requirements

In the following we list the requirements of the Error Handling (EH) strategy. The term platform
refers to the usage environment of the error handler. There can be various platforms,
e.g. for regular operations, testing, different OS etc.

### Functional

1. The EH shall support error codes.
    - configurable on a platform basis
    - errors from different platforms shall not be usable (checked at compile time)
    - uniqueness is not enforced (can be done by the layer above if desired)
    - error codes shall be optional (if none is used, it is mapped to some default error code)
1. The EH shall support multiple error levels.
    - the individual error levels are defined by the platform
    - there has to be at least a FATAL error level (most severe error level)
    - the error levels shall be ordered from least severe to most severe
    - if the code requires an ERROR level that is not supported by the platform it shall not compile
1. The EH shall support a non-recoverable error handling strategy in case of FATAL errors.
    - non-recoverable errors shall call terminate after optional handling and logging code has been executed
1. The EH shall support a recoverable error handling strategies for non-FATAL errors.
    - recoverable error handling shall support execution of sufficiently general user actons (callbacks)
1. The detection mechanism of the EH shall allow logging the error.
    - user defined-logging shall be supported
    - there shall be a default implementaton for logging
1. The EH shall allow to trace the error to its source location (file, function, line).
1. The EH shall be disableable for non-FATAL errors.
    - disabling shall be possible up to some level at compile time; this level shall be non-FATAL
    - can be used if there is sufficient trust in the system to disable e.g. warning checks
    - by default all levels defined by the platform are enabled
    - disabling shall cause the affected error logic in the code to cause no runtime overhead

### Technical Constraints

1. The EH shall be uniquely defined for each platform.
    - only one error handler can be active for a platform at any time
    - the EH shall not be changed while the system runs
    - the EH must be configurable at runtime in the init phase
      (for tests, but it might be sufficient to change it at compile time if only required for tests)
    - for tests it must be possible to override the FATAL error reaction causing termination
    - If there is no error handler set the system shall not run (check at the very beginning or enforc)
2. The EH shall not use any exceptions.
    - this excludes user callbacks (impossible to enforce)
3. The EH shall not use dynamic memory.
    - this excludes user callbacks (impossible to enforce)
4. The EH shall be non-blocking.
    - recoverable errors shall eventually pass back control to the invoking function
    - cannot be enforced with user callbacks (the user is responsible to use non-blocking calls only)
5. The EH shall be highly-visible in the code to be able to distinguish it from ordinary function calls.
    - allows simple search for error handling locations in the code
    - if implemented via macros it can easily be disabled
6. The EH may depend on logging to some extent.
    - only if it is supposed to use the logging capabilities in some platform implementation
    - the logger cannot use the (complete) EH due to this

## API Proposal

The implementation can use a combination of templates and macros to achieve the goals regarding usability,
configurability and perfomance.

### Recoverable Error Handling API

Assume we already have checked for some error condition and determined that we have to raise an error, i.e.
it is not sufficient to propagate the error to some upper layer. This is a design decision, especially for
non-FATAL errors.
Then we can provide the ability to raise the error like so:

`RAISE_ERROR(error_code, error_level)`

We can define default values for the final arguments
By default `error_level` is `FATAL` and hence could be optionally omitted.

Optionally we can consider passing some function (functor) that will be invoked in addition,
but this is basically syntactic sugar at this point as we already have determined that an error occured
and could call this functor directly. The ability to pass this function makes the implementation more difficult
if sufficent generality is desired (optional arguments etc.).

The installed error handler can now decide how to proceed with the raised error and delegate to logging etc,
shutdown code etc. Providing this call is useful to allow specific shutdown code which otherwise would require a lookup
in some error code table (which can still be done if required).

The only requirment is that for FATAL errors the `RAISE_ERROR` invocation shall never return and lead to termination.

It is also possible to provide no `error_code` at all, which leads t a generic FATAL error:

`RAISE_ERROR()`.

A further conisderation is allowing custom messages (in principle not necessary if we allow custom handler functions)
which are delegated to the handling code. The handling code can then decide to log them or otherwise incorporate
it into the reporting code.

`RAISE_ERROR(error_code, FATAL, "Mutex corrupted")`.

The problem of various optional arguments is to define a proper argument order and, if macros are used,
the requirement of variadic macros. It is generally not possible to leave out arguments in the middle
without defining an entirely different macro or function.
The macro wrappers should be very thin and directy delegate to a template function that enables
the compiler to optimize the code.

### Non-recoverable Error Handling API

Non recoverable errors could be raised with the general API as well as

`RAISE_ERROR(error_code, FATAL, optional_message)`

This assumes we have checked for some error `condition` first. We can incorporate the check in another macro
and arrive at:

`ASSERT(!condition, error_code, optional_message)`

This is mainly syntactic sugar to increase readability and state intent in a more concise way.
We can use both notations. Notice that this always operates at FATAL level and must be always active
since there is no notion how we can continue normally from there (otherwise it would not be a FATAL error).

Again optional arguments lead to a more involved implementation. Providing an optional handler is even more useful here
(as the check is happens in the macro) and can look like this:

`ASSERT(!condition, error_code, handler, optional_handler_args)`

If the handler is not invocable with provided arguments (if any) then the compilation will fail.
Unless we define another macro for the default error code, `error_code` must be provided.
It is not advisable to manage this in the handler for consistency and to allow search for the error codes.

### Generalization of ASSERT to Recoverable Errors

If we want to support recoverable errors with a similar syntax to `ASSERT` we can define

`EXPECT(condition, error_code, error_level, handler, optional_handler_args)`

This is similar to Goofle Test terminology and basically states that we expect some `condition` to be true
but do not necessarily terminate if it is not (depending on the `error_level`).
If it is not true we raise an error of the required level.

Hence
`EXPECT(condition, error_code, FATAL, handler, optional_handler_args)`

is equivalent to

`ASSERT(condition, error_code, handler, optional_handler_args)`.

If we do not need to call a handler and just provide some error message we can define

`EXPECT(condition, error_code, error_level, error_message)`

or even omit it entirely

`EXPECT(condition, error_code, error_level)`

We have the possibility to specify at compile time the minimum error level we intend to handle to e.g. exclude warnings.
This can be done without causing overhead for the affected checks. Assuming there are the levels
`WARNING`, `RECOVERABLE_ERROR`, `FATAL` we can suppress all checks for `WARNING` in a way that not even
`condition` is evaluated in

`EXPECT(condition, error_code, WARNING)`,

i.e. achieve a zero-overhead abstraction. This is useful if `WARNING`s are indeed no problem that compromise the
ability to continue but instead unusal system states we encounter.

### Replacement of Expects and Ensures

`ASSERT` should replace `Expects` and `Ensures`. In particular there is no use to distinguish between them unless it is
possible to check the conditions **before** the function is actually invoked or immediately **after** it returns.
It is not possible with the capabilities of the current C++ language to specify true preconditions and postconditions
conveniently (e.g. without wrapping all functions manually or with some automation),
hence both are basically custom asserts.

This is consistent with the C++ error handling proposal <https://doc.bccnsoft.com/docs/cppreference2018/en/cpp/language/attributes/contract.html>
which is encourages to use `assert` inside a function body.
The proposed `ASSERT` is more general except in one regard: it cannot be disabled by design. Disabling it could easily
be made possible, but this would lead to problems if thefailure conditions where ever encountered to be true
since it would be impossible to safely proceed.

## Error Propagation

Idiomatic propagation of errors with `expected`, `optional` or return codes is not in scope here ad will be covered elsewhere.

## Open Points

1. Design considerations of functionality such as general handler code provided by the user
1. Optional arguments if the proposed macro API path is chosen
1. Replacement strategy and definition of the active error handler will be similar to the new logger (platforms specific)
1. Prototype implementation and detailed design
1. Error propataion with `expected` is another issue but does not concern the error handler invocation
1. Replacement of noexcept to allow exceptions in client code (e.g. types, callbacks) is not in scope here
1. Macro naming should be close to Google Test but ideally not the same to ease searches.
   Currently it is just a rough proposal.

TBD
