# Logging

## Summary and problem description

Logging is a crucial part of a framework since it informs the developer and
user of the framework about anomalies and helps with debugging by providing
contextual information.

Additionally, when integrated into a separate framework, the logging should be
forwarded in order to have a single logging infrastructure.

The logging should also be performant and not allocate memory. Ideally it should
be possible to disable it at compile time and let the compiler optimize it away.

It shall also be possible to alter the behavior of the logging via environment
variables, e.g. the log level.

## Terminology

### Log levels and how to use them

| LogLevel | Usage |
|----------|-------|
| FATAL    | For fatal an non-recoverable errors which essentially leads to a termination of the program. |
| ERROR    | For severe but recoverable errors. |
| WARN     | For cases when something unintentional happens but not considered as severe error. |
| INFO     | For important status information a user should be aware of. |
| DEBUG    | Anything that is helpful for debugging. |
| TRACE    | Any comment could be replaced by logging call with log level trace. |

The log levels `FATAL`, `ERROR` and `WARN` should not be used independent but in
combination with the error handler.

## Design

### Considerations

1. Similar to other iceoryx constructs, heap usage is forbidden. This also means
   that `std::string` and `std::stringstream` cannot be used.

2. Since the logger shall be used by the `iceoryx_hoofs` itself, the base
   implementation can also not make use of functionality in e.g. `iox::cxx` or
   `iox::posix`

3. To have a minimal runtime overhead, string formatting shall only happen when
   the log level is above the output threshold. This lazy evaluation shall also
   apply to other potentially expensive operations like the result of a function
   call.

4. Additionally, the minimal log level should be configurable by a compile time
   switch and everything below this log level should be compiled to no-ops. With
   this, developer can make excessive use of the most verbose log level and
   disable it by default. When debugging, this can be turned on and help to find
   the root of the problem.

5. There shall also be the possibility to alter the behavior of the logger via
   environment variables. This can be functionality like setting the log level or
   deciding whether the timestamps or file names shall be part of a log message.

6. Since iceoryx is often used as transport layer in a higher level framework, it
   shall be possible to replace the default logger at runtime and delegate the
   log messages to the logger of such a framework. To prevent misuse, this can
   only be done up to a specific time. Afterwards an error message shall be
   logged in the new and old logger and the error handler shall be called with a
   moderate error level.

7. The default logger can be replaced via the platform abstraction in order to
   use the dedicated logger for log messages which are emitted before it could
   be replaced at runtime, e.g. from global objects before `main`.

8. Additionally, there shall be a compile time option to forward all log messages
   independent of the log level threshold for cases where the logger framework
   does the filtering itself.

9. For tests the default logger shall be replaced to suppress the log messages in
   passed tests but output them for failed tests. This keeps the noise low but
   still provides useful information when needed. Additionally there shall be an
   environment variable to circumvent the suppression when desired.

### Solution

#### Class diagram

![logging class diagram](../website/images/logging_classes.svg)

#### Macro with lazy evaluation

The buildup of the log message is only done when the log level is above the output
threshold. This is accomplished by a macro with an incomplete if-statement.

```cpp
#define LAZY() if (cond)

LAZY() expensiveFunctionCall();
```

In the example above `expensiveFunctionCall` is only executed when `cond` is `true`.
If `cond` is a compile time constant set to `false`, the whole statement is compiled
to a no-op and optimized away.

This is the log macro with lazy evaluation

```cpp
#define IOX_LOG_INTERNAL(file, line, function, level)          \
    if ((level) <= iox::log::Logger::minimalLogLevel()         \
        && (iox::log::Logger::ignoreLogLevel()                 \
            || (level) <= iox::log::Logger::getLogLevel()))    \
    iox::log::LogStream(file, line, function, level).self()

#define IOX_LOG(level) IOX_LOG_INTERNAL(__FILE__, __LINE__, __FUNCTION__, iox::log::LogLevel::level)
```

With `minimalLogLevel` and `ignoreActiveLogLevel` being static constexpr functions
the compiler will optimize this either to `if (false) iox::log::LogStream(...)`
and finally completely away or
`if ((level) <= iox::log::Logger::activeLogLevel()) iox::log::LogStream(...)`.
The minimal log level check is intended to fully optimize away a log statement
and the ignore active log level check to always forward the log message to the
logger, independent of the active log level.

The `self()` invocation is used to create an lvalue reference to the `LogStream`
object. This eases the implementation of logging support for custom types since
`IOX_LOG(INFO) << myType;` would require to implement an overload with a rvalue
`LogStream` reference but `IOX_LOG(INFO) << "#### " << myType;` requires a
lvalue reference.

The `IOX_LOG` macro is intended for general use and the `IOX_LOG_INTERNAL` for
special cases when file, line and function are supplied from other places than
the log macro invocation, like the `Expects` and `Ensures` macros.

Although the macro contains an incomplete if-statement, the `LogStream` object at
the end makes it safe to use since the compiler will complain if something else
than a streaming operator or semicolon is used.

#### Behavior before calling initLogger

In order to have log messages before `initLogger` is called, the default logger
is used with `LogLevel::INFO`. It is up to the implementation of the default
logger what to do with these messages. For iceoryx the default logger is the
`ConsoleLoger` (this can be changed via the platform abstraction) which will
print the log messages to the console.

#### Replacing the logger at runtime

The default console logger can be replaced at runtime to a custom logger which
can be used to forward the log messages to a logger of another framework.

This is done by deriving from the base `Logger` class and implementing the pure
virtual `setupNewLogMessage` and `flush` methods. Finally, the derived logger
must be activated by calling the static `Logger::activeLogger` function and
passing the new logger as argument to the function. The logger must have a static
lifetime and should therefore be place in the data segment.

The call to `iox::log::initLogger` will finalize the option to replace the logger
at runtime. Further efforts to replace the logger will call the error handler
with a `MODERATE` error level and a log message to the old and new logger

See also the code example to [create a custom logger](#creating-a-custom-logger).

![logger runtime replacement](../website/images/logger_runtime_replacement.svg)

#### Replacing the default logger for a platform

Each platform must specify the following aliases in the `platform_settings.hpp`
header.

```cpp
using LogLevel = pbb::LogLevel;
using pbb::asStringLiteral;

using Logger = pbb::Logger;
using DefaultLogger = pbb::ConsoleLogger;
using TestingLoggerBase = pbb::ConsoleLogger;
```

With `LogLevel` enum being the log level enum with the values defined in the
[class diagram](#class-diagram). The `Logger` must be a class specifying the
interface from the class diagram and `DefaultLogger` and `TestingLoggerBase`
are the actual implementations which are used by default in the library and for
the tests.

It is recommended to derive from `pbb::Logger` to create a custom
logger but it would also be possible to recreate it from scratch as long as the
interface is satisfied.

The implementation of a custom default logger is similar to the logger which
can be replaced at runtime, except the static `init` function. Since the default
logger must be initialized by the `iox::log::initLogger` call, it is not
recommended to provide a public init function. If some initialization is needed,
the `initLoggerHook` virtual function can be overloaded. This is called after
the base logger is initialized.

![logger compile time replacement](../website/images/logger_compile_time_replacement.svg)

#### Dedicated logger for testing

In order to have quiet tests, the logging output shall be suppressed for passed
tests and printed to the console for failed tests.

For this purpose a `TestingLogger` shall be supplied and initialized right after
`::testing::InitGoogleTest(&argc, argv)` and before `RUN_ALL_TESTS()`

This are the classes involved in the testing setup:

![logger testing classes](../website/images/logger_testing_classes.svg)

This is the sequence diagram of the setup of the testing logger:

![logger testing sequence](../website/images/logger_testing_sequence.svg)

#### Environment variables

The behavior of the logger can be altered via environment variables and the
`initLogger` function. Calling this function without arguments, it will check
whether the environment variable `IOX_LOG_LEVEL` is set or use `LogLevel::INFO`
as log level. To have a different fallback log level, the `logLevelFromEnvOr`
function can be used, e.g.

```cpp
iox::log::initLogger(iox::log::logLevelFromEnvOr(iox::log::LogLevel::DEBUG));
```

If the logger shall not be altered via environment variables, `initLogger` must
be called with the fitting log level.

For the `TestingLogger` there is an additional environment variable called
`IOX_TESTING_ALLOW_LOG`. This enables the printing of the log messages for all
tests instead of only for failed ones.

| Environment variable | Allowed values |
|--------------|-----------|
| IOX_LOG_LEVEL | off, fatal, error, warn, info, debug, trace |
| IOX_TESTING_ALLOW_LOG | on, off |

#### Thread local storage

The `Logger` base class is using thread local storage to provide a buffer for
each thread. In case thread local storage is not desired, the logger must be
re-implemented via the platform abstraction.

### Code example

#### Using the default console logger

```cpp
#include "iceoryx_hoofs/log/logging.hpp"

int main()
{
    iox::log::initLogger(iox::log::logLevelFromEnvOr(iox::log::LogLevel::DEBUG));

    IOX_LOG(DEBUG) << "Hello World";

    return 0;
}
```

#### Add logging support for custom types

```cpp
#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/log/logstream.hpp"
#include <cstdint>

struct MyType
{
    uint16_t a {0};
    uint16_t b {0};
};

iox::log::LogStream& operator<<(iox::log::LogStream& stream, const MyType& m)
{
    stream << "MyType { a = " << m.a << "; b = " << m.b << " }";
    return stream;
}

int main()
{
    MyType m;
    IOX_LOG(INFO) << m;

    return 0;
}
```

#### Creating a custom logger

```cpp
#include "iceoryx_hoofs/log/logger.hpp"
#include "iceoryx_hoofs/log/logging.hpp"

class MyLogger : public iox::log::Logger
{
  public:
    static void init()
    {
        static MyLogger myLogger;
        iox::log::setActiveLogger(&myLogger);
        iox::log::initLogger(iox::log::logLevelFromEnvOr(iox::log::LogLevel::INFO));
    }

  private:
    void setupNewLogMessage(const char*, const int, const char*, iox::log::LogLevel logLevel) override
    {
        switch(logLevel) {
            case iox::log::LogLevel::FATAL:
                logString("💀: ");
                break;
            case iox::log::LogLevel::ERROR:
                logString("🙈: ");
                break;
            case iox::log::LogLevel::WARN:
                logString("🙀: ");
                break;
            case iox::log::LogLevel::INFO:
                logString("💘: ");
                break;
            case iox::log::LogLevel::DEBUG:
                logString("🐞: ");
                break;
            case iox::log::LogLevel::TRACE:
                logString("🐾: ");
                break;
            default:
                logString("🐔: ");
        }
    }

    void flush() override {
        puts(m_buffer);
        m_bufferWriteIndex = 0;
    }
};

int main()
{
    MyLogger::init();

    IOX_LOG(FATAL) << "Whoops ... look, over there is a dead seagull flying!";
    IOX_LOG(ERROR) << "Oh no!";
    IOX_LOG(WARN) << "It didn't happen!";
    IOX_LOG(INFO) << "All glory to the hypnotoad!";
    IOX_LOG(DEBUG) << "I didn't do it!";
    IOX_LOG(TRACE) << "Row row row your boat!";

    return 0;
}
```

## Open issues

- do we need to change the log level after `initLogger`
- do we want a `IOX_LOG_IF(cond, level)` macro
- shall the TestingLogger register signals to catch SIGTERM, etc. and print the
  log messages when the signal is raised? It might be necessary to wait for the
  error handling refactoring before this can be done
