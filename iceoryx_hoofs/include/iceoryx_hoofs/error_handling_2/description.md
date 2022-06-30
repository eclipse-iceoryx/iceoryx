# Brief explanation

General goal: evaluate as much at compile time as possible and minimize runtime overhead.
Accomplished by meta-programming using the C++ type system.

Obviously the occurrence of an error is known at runtime only and hence trhe optimization is limited
to only executing handler code that if required and (almost) nothing if no error occurs.

Location of the folders platform and module is preliminary and for structuring puproses only.

## platform

- defined once for a compilation unit
- all compilation units linked together to one library should probably always share the platform (not strictly required though)
- requires implementation of error_handling.hpp and error_levels.hpp (with some constraints)
- error_levels.hpp defines the error levels used by the platform
- error_handling.hpp defines the reaction for all error levels
- error_handling.hpp defines the general termination routine
- it is possible to define a test specific platform implementation to verify certain errors occur as expected
- it is possible to deactivate errors in the platform (causes them being compiled to nothing)
- assumption: platform is only supposed to be changed rarely and by experienced developers (it defines behavior of low-level primitives in addition to error handling strategy)

## module

- has a unique module id (!=0) defined by the developer
- defines error codes in the module (single enum class per module id)
- error codes must be consecutive 0, 1, 2, ...
- multiple modules per translation unit are possible (in contrast to platform)
- for us a module would be e.g. posh or hoofs (more fine-grained modules are possible)

## Errors

- can be unspecific, i.e. no module (id = 0) and arbitrary code
- specific with strongly typed code and module id
- always have a level (implicit is FATAL which always exists)
- FATAL errors lead to execution of the handling code followed by a termination routine
- handling code an be a NO-OP, reporting etc.
- throwing exceptions in handlers is possible with restrictions (useful in e.g. the test platform)

## API

- requires macros for source location
- should be safe to use as they are only a thin wrapper around a generic API
- see test cases for some usage demonstration (to be refined)
- syntactic sugar: IOX_ASSERT, IOX_DEBUG_ASSERT, custom messages, calling context dependent handler functions etc. (partly work in progress)

## Limitations

- errors should not be raised in destructors (problematic by design)
- when a destructor must raise an error it has to be FATAL
- this is even more important if the handler code is throwing exceptions

## Configurability

1. error levels can be defined within the platform (FATAL is mandatory)
2. error codes can be defined within modules
3. conversion from error codes to errors must be provided
4. errors only have to satisfy a minimal interface (provide module id and error code) but are otherwise flexible
5. logging always happens according to the log level (logger needs to be used when ready)
6. error is delegated further to platform specific handling (does nothing by default)
7. platform specific handling can be implemented in a way that the reaction can be dynamically changed at runtime (useful for e.g. tests but incurs a performance hit)
    - handling does not have to change dynamically at runtime
    - dynamic change can be finalized to prohibit change after a certain phase (e.g. init)
