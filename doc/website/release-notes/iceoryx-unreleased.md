# iceoryx vx.x.x

## [vx.x.x](https://github.com/eclipse-iceoryx/iceoryx/tree/vx.x.x) (xxxx-xx-xx) <!--NOLINT remove this when tag is set-->

[Full Changelog](https://github.com/eclipse-iceoryx/iceoryx/compare/vx.x.x...vx.x.x) <!--NOLINT remove this when tag is set-->

**Features:**

- Add bazel asan, usan, tsan build config [#1547](https://github.com/eclipse-iceoryx/iceoryx/issues/1547)
- Add bazel clang build config [#1998](https://github.com/eclipse-iceoryx/iceoryx/issues/1998)
- Add `command_line.hpp` which contains a macro builder to parse command line arguments quickly and safely [#1067](https://github.com/eclipse-iceoryx/iceoryx/issues/1067)
- optional inherits from FunctionalInterface, adds .expect() method [\#996](https://github.com/eclipse-iceoryx/iceoryx/issues/996)
- Add clear method for `iox::string` [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- Add at method and operator[] for `iox::string` [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- expected inherits from FunctionalInterface, adds .expect() method [\#996](https://github.com/eclipse-iceoryx/iceoryx/issues/996)
- Add CI check of used headers against a list [\#1252](https://github.com/eclipse-iceoryx/iceoryx/issues/1252)
- Add insert method for `iox::string` [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- Extend compare method of `iox::string` to compare additionally with std::string and char array [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- Add compare method for `iox::string` and chars [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- Refactor semaphore [\#751](https://github.com/eclipse-iceoryx/iceoryx/issues/751)
    - Introduce `UnnamedSemaphore`
    - Introduce `NamedSemaphore`
    - Remove old `Semaphore`
- Extend `concatenate`, `operator+`, `unsafe_append` and `append` of `iox::string` for chars [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- Extend `unsafe_append` and `append` methods of `iox::string` for `std::string` [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- The iceoryx development environment supports multiple running docker containers [\#1410](https://github.com/eclipse-iceoryx/iceoryx/issues/1410)
- Use builder pattern in FileLock [\#1036](https://github.com/eclipse-iceoryx/iceoryx/issues/1036)
    - Add the ability to adjust path and file permissions of the file lock
- Create convenience macro for `NewType` [\#1425](https://github.com/eclipse-iceoryx/iceoryx/issues/1425)
- Add posix thread wrapper [\#1365](https://github.com/eclipse-iceoryx/iceoryx/issues/1365)
- Apps send only the heartbeat when monitoring is enabled in roudi [\#1436](https://github.com/eclipse-iceoryx/iceoryx/issues/1436)
- Support [Bazel](https://bazel.build/) as optional build system [\#1542](https://github.com/eclipse-iceoryx/iceoryx/issues/1542)
- Support user defined platforms with cmake switch `-DIOX_PLATFORM_PATH` [\#1619](https://github.com/eclipse-iceoryx/iceoryx/issues/1619)
- Add equality and inequality operators for `iox::variant` and `iox::expected` [\#1751](https://github.com/eclipse-iceoryx/iceoryx/issues/1751)
- Implement UninitializedArray [\#1614](https://github.com/eclipse-iceoryx/iceoryx/issues/1614)
- Implement BumpAllocator [\#1732](https://github.com/eclipse-iceoryx/iceoryx/issues/1732)
- Expand cmake configuration options to enable reducing shared memory consumption. [\#1803](https://github.com/eclipse-iceoryx/iceoryx/issues/1803)
- Implement PolymorphicHandler [\#1640](https://github.com/eclipse-iceoryx/iceoryx/issues/1640)
- Implement SemanticString as base class for strong string types. [\#1942](https://github.com/eclipse-iceoryx/iceoryx/issues/1942)
- Implement UserName as strong string type to represent posix user names. [\#1942](https://github.com/eclipse-iceoryx/iceoryx/issues/1942)
- Implement FileName, GroupName, Path, FilePath as strong string types. [\#1942](https://github.com/eclipse-iceoryx/iceoryx/issues/1942)
- Add string::unchecked_at to access character without bound checks. [\#1942](https://github.com/eclipse-iceoryx/iceoryx/issues/1942)
- Add posix::FileManagementInterface to offer common operations like ownership/permission handling to all file descriptor based constructs. [\#1952](https://github.com/eclipse-iceoryx/iceoryx/issues/1952)
- Implement dereference operator for smart_lock::Proxy [\#1966](https://github.com/eclipse-iceoryx/iceoryx/issues/1966)
- `NewType` supports arithmetic operations and loops [\#1554](https://github.com/eclipse-iceoryx/iceoryx/issues/1554)
- Add `iox::span` [\#180](https://github.com/eclipse-iceoryx/iceoryx/issues/180)
- Implement custom error reporting API [\#1032](https://github.com/eclipse-iceoryx/iceoryx/issues/1032)
- Implement iceoryx platform for FreeRTOS [#1982](https://github.com/eclipse-iceoryx/iceoryx/issues/1982)
- Extend 'iceperf' with 'WaitSet' [#2003](https://github.com/eclipse-iceoryx/iceoryx/issues/2003)

**Bugfixes:**

- FreeBSD CI build is broken [\#1338](https://github.com/eclipse-iceoryx/iceoryx/issues/1338)
- High CPU load in blocked publisher is reduced by introducing smart busy loop waiting (adaptive_wait) [\#1347](https://github.com/eclipse-iceoryx/iceoryx/issues/1347)
- Compile Error : iceoryx_dds/Mempool.hpp: No such file or directory [\#1364](https://github.com/eclipse-iceoryx/iceoryx/issues/1364)
- RPATH is correctly set up for all libraries and binaries. [\#1287](https://github.com/eclipse-iceoryx/iceoryx/issues/1287)
- Wrong memory order in concurrent::FIFO [\#1396](https://github.com/eclipse-iceoryx/iceoryx/issues/1396)
- Iceoryx libraries weren't compiled with `-fPIC` as position independent code [#\879](https://github.com/eclipse-iceoryx/iceoryx/issues/879)
- Restrict runtime (application) names to valid file names to solve failures in the underlying posix constructs [#\1419](https://github.com/eclipse-iceoryx/iceoryx/issues/1419)
- CMake config assumes relative `CMAKE_INSTALL_LIBDIR` [\#1393](https://github.com/eclipse-iceoryx/iceoryx/issues/1393)
- Build error on certain versions of Windows/Visual Studio [\#1476](https://github.com/eclipse-iceoryx/iceoryx/issues/1476)
- Fix INTERFACE_INCLUDE_DIRECTORIES in CMake [\#1481](https://github.com/eclipse-iceoryx/iceoryx/issues/1481)
- The testing libs are broken for in source tree usage [\#1528](https://github.com/eclipse-iceoryx/iceoryx/issues/1528)
  - This bug was not part of a release but introduce during the v3 development
- Add "inline" keyword to smart_lock method implementation [\#1551](https://github.com/eclipse-iceoryx/iceoryx/issues/1551)
- Fix RouDi crash due to uninitialized `ServiceRegistry` chunk [\#1575](https://github.com/eclipse-iceoryx/iceoryx/issues/1575)
- Add check in `cxx::unique_ptr::get` to avoid `nullptr` dereferencing [\#1571](https://github.com/eclipse-iceoryx/iceoryx/issues/1571)
- Pass `CleanupCapacity` to underlying `iox::function` in `ScopeGuard` (formerly known as `cxx::GenericRAII`) [\#1594](https://github.com/eclipse-iceoryx/iceoryx/issues/1594)
- Add check in `RelativePointer::get` to avoid `nullptr` dereferencing [\#1596](https://github.com/eclipse-iceoryx/iceoryx/issues/1596)
- iceoryx_posh_testing cannot find iceoryx_hoofs_testing in CMake [\#1602](https://github.com/eclipse-iceoryx/iceoryx/issues/1602)
- locking_policy.cpp calls error handler without log message [\#1609](https://github.com/eclipse-iceoryx/iceoryx/issues/1609)
- Implement destructor, copy and move operations in `iox::stack` [\#1469](https://github.com/eclipse-iceoryx/iceoryx/issues/1469)
- `gw::GatewayGeneric` sometimes terminates discovery and forward threads immediately [\#1666](https://github.com/eclipse-iceoryx/iceoryx/issues/1666)
- `m_originId` in `mepoo::ChunkHeader` sometimes not set [\#1668](https://github.com/eclipse-iceoryx/iceoryx/issues/1668)
- Remove `cxx::unique_ptr::reset` [\#1655](https://github.com/eclipse-iceoryx/iceoryx/issues/1655)
- CI uses outdated clang-format [\#1736](https://github.com/eclipse-iceoryx/iceoryx/issues/1736)
- Avoid UB when accessing `iox::expected` [\#1750](https://github.com/eclipse-iceoryx/iceoryx/issues/1750)
- Fix double move in `vector::emplace` [\#1823](https://github.com/eclipse-iceoryx/iceoryx/issues/1823)
- Default roudi_config.toml path is not used [\#1826](https://github.com/eclipse-iceoryx/iceoryx/issues/1826)
- `WaitSet::wait` returns if data was send before `WaitSet::attachState(.., State::HAS_{DATA, REQUEST, RESPONSE})` [\#1855](https://github.com/eclipse-iceoryx/iceoryx/issues/1855)
- Provide a better error message when attempting to create a shared memory in read-only mode
  [\#1821](https://github.com/eclipse-iceoryx/iceoryx/issues/1821)
- Can not build iceoryx with gcc 9.4 [\#1871](https://github.com/eclipse-iceoryx/iceoryx/issues/1871)
- Update iceoryx_integrationtest package to use ROS2 Humble [\#1906](https://github.com/eclipse-iceoryx/iceoryx/issues/1906)
- Fix potential memory leak in `iox::stack` [\#1893](https://github.com/eclipse-iceoryx/iceoryx/issues/1893)
- Make `MAX_USER_NAME_LENGTH` and `MAX_GROUP_NAME_LENGTH` platform-dependent [\#1919](https://github.com/eclipse-iceoryx/iceoryx/issues/1919)
- Fix milliseconds in log timestamps [\#1932](https://github.com/eclipse-iceoryx/iceoryx/issues/1932)
- Alias `invoke_result` to correct implementation based on C++ version [\#1934](https://github.com/eclipse-iceoryx/iceoryx/issues/1934)
- Fix undefined behaviour in `publishCopyOf` and `publishResultOf` [\#1963](https://github.com/eclipse-iceoryx/iceoryx/issues/1963)
- ServiceDescription `Interfaces` and `INTERFACE_NAMES` do not match [\#1977](https://github.com/eclipse-iceoryx/iceoryx/issues/1977)
- Implement and test nullptr check in c binding [\#1106](https://github.com/eclipse-iceoryx/iceoryx/issues/1106)
- Fix `expected<void, Error>` is unusable due to `final` [\#1976](https://github.com/eclipse-iceoryx/iceoryx/issues/1976)

**Refactoring:**

- Separate module specific errors from `iceoryx_hoofs` [\#1099](https://github.com/eclipse-iceoryx/iceoryx/issues/1099)
  - Move test specific code to `ErrorHandlerMock` and templatize `setTemporaryErrorHandler()`
  - Create separate error enum for each module
- Use `GTEST_FAIL` and `GTEST_SUCCEED` instead of `FAIL` and `SUCCEED` [\#1072](https://github.com/eclipse-iceoryx/iceoryx/issues/1072)
- posix wrapper `SharedMemoryObject` is silent on success [\#971](https://github.com/eclipse-iceoryx/iceoryx/issues/971)
- Remove creation design pattern class with in place implementation [\#1036](https://github.com/eclipse-iceoryx/iceoryx/issues/1036)
  - posix wrapper `SharedMemoryObject` uses builder pattern instead of creation
  - Builder pattern extracted from `helplets.hpp` into `iox/builder.hpp`
- Uninteresting mock function calls in tests [\#1341](https://github.com/eclipse-iceoryx/iceoryx/issues/1341)
- `cxx::unique_ptr` owns deleter, remove all deleter classes [\#1143](https://github.com/eclipse-iceoryx/iceoryx/issues/1143)
- Remove `iox::posix::Timer` [\#337](https://github.com/eclipse-iceoryx/iceoryx/issues/337)
- Refactor service discovery tests [/#1065](https://github.com/eclipse-iceoryx/iceoryx/issues/1065)
  to increase comprehension and cover more test cases
- Remove usage of `std::function` [\#831](https://github.com/eclipse-iceoryx/iceoryx/issues/831)
- Replace `MethodCallback` with `iox::function` [\#831](https://github.com/eclipse-iceoryx/iceoryx/issues/831)
- Remove null-ability `iox::function_ref` [\#1104](https://github.com/eclipse-iceoryx/iceoryx/issues/1104)
- Remove implicit conversion from `iox::expected` to `iox::optional` [\#1196](https://github.com/eclipse-iceoryx/iceoryx/issues/1196)
- Remove AtomicRelocatablePointer [\#1512](https://github.com/eclipse-iceoryx/iceoryx/issues/1512)
- `SignalHandler` returns an `iox::expected` in `registerSignalHandler` [\#1196](https://github.com/eclipse-iceoryx/iceoryx/issues/1196)
- Remove the unused `PosixRights` struct [\#1556](https://github.com/eclipse-iceoryx/iceoryx/issues/1556)
- Move quality level 2 classes to new package `iceoryx_dust` [\#590](https://github.com/eclipse-iceoryx/iceoryx/issues/590)
- Remove unused classes from `iceoryx_hoofs` [\#590](https://github.com/eclipse-iceoryx/iceoryx/issues/590)
  - `cxx::PoorMansHeap`
  - Other `internal` classes
- Cleanup helplets [\#1560](https://github.com/eclipse-iceoryx/iceoryx/issues/1560)
- Move package `iceoryx_dds` to [separate repository](https://github.com/eclipse-iceoryx/iceoryx-gateway-dds) [\#1564](https://github.com/eclipse-iceoryx/iceoryx/issues/1564)
- Set `SOVERSION` with project major version for shared libraries in CMake [\#1308](https://github.com/eclipse-iceoryx/iceoryx/issues/1308)
- Monitoring feature of RouDi is now disabled by default [\#1580](https://github.com/eclipse-iceoryx/iceoryx/issues/1580)
- Rename `cxx::GenericRAII` to `iox::ScopeGuard` [\#1450](https://github.com/eclipse-iceoryx/iceoryx/issues/1450)
- Rename `algorithm::max` and `algorithm::min` to `algorithm::maxVal` and `algorithm::minVal` [\#1394](https://github.com/eclipse-iceoryx/iceoryx/issues/1394)
- Extract `iceoryx_hoofs/platform` into separate package `iceoryx_platform` [\#1615](https://github.com/eclipse-iceoryx/iceoryx/issues/1615)
- `cxx::unique_ptr` is no longer nullable [\#1104](https://github.com/eclipse-iceoryx/iceoryx/issues/1104)
- Use builder pattern in mutex [\#1036](https://github.com/eclipse-iceoryx/iceoryx/issues/1036)
- Change return type of `vector::erase` to bool [\#1662](https://github.com/eclipse-iceoryx/iceoryx/issues/1662)
- `ReleativePointer::registerPtr` returns `iox::optional` [\#605](https://github.com/eclipse-iceoryx/iceoryx/issues/605)
- `iox::function` is no longer nullable [\#1104](https://github.com/eclipse-iceoryx/iceoryx/issues/1104)
- Rename `BaseRelativePointer` to `UntypedRelativePointer` [\#605](https://github.com/eclipse-iceoryx/iceoryx/issues/605)
- Prevent building GoogleTest when `GTest_DIR` is defined [\#1758](https://github.com/eclipse-iceoryx/iceoryx/issues/1758)
- Refactor `iceoryx_posh_testing` library into own CMakeLists.txt [\#1516](https://github.com/eclipse-iceoryx/iceoryx/issues/1516)
- Change return type of `cxx::variant::emplace_at_index` and `emplace` to void [\#1394](https://github.com/eclipse-iceoryx/iceoryx/issues/1394)
- Replace uses of `std::cout`, `std::cerr` with the iceoryx logger [\#1756](https://github.com/eclipse-iceoryx/iceoryx/issues/1756)
- Move `IOX_NO_DISCARD`, `IOX_FALLTHROUGH` and `IOX_MAYBE_UNUSED` to `iceoryx_platform` [\#1726](https://github.com/eclipse-iceoryx/iceoryx/issues/1726)
- Move `cxx::static_storage` from `iceoryx_hoofs` to `iceoryx_dust` [\#1732](https://github.com/eclipse-iceoryx/iceoryx/issues/1732)
- Remove `algorithm::uniqueMergeSortedContainers` from `algorithm.hpp`
- Move `std::string` conversion function to `iceoryx_dust` [\#1612](https://github.com/eclipse-iceoryx/iceoryx/issues/1612)
- The posix call `unlink` is directly used in `UnixDomainSocket` [\#1622](https://github.com/eclipse-iceoryx/iceoryx/issues/1622)
- Wrap all C calls in posixCall in IntrospectionApp [\#1692](https://github.com/eclipse-iceoryx/iceoryx/issues/1692)
- Move `std::chrono` dependency to `iceoryx_dust` [\#536](https://github.com/eclipse-iceoryx/iceoryx/issues/536)
- Move `std::string` dependency from `iox::string` to `std_string_support.hpp` in `iceoryx_dust` [\#1612](https://github.com/eclipse-iceoryx/iceoryx/issues/1612)
- Better align `iox::expected` with `std::expected` [\#1969](https://github.com/eclipse-iceoryx/iceoryx/issues/1969)
- Use logger for "RouDi is ready for clients" message [\#1994](https://github.com/eclipse-iceoryx/iceoryx/issues/1994)

**Workflow:**

- Remove hash from the branch names [\#1530](https://github.com/eclipse-iceoryx/iceoryx/issues/1530)
- Automate check for test cases to have UUIDs [\#1540](https://github.com/eclipse-iceoryx/iceoryx/issues/1540)
- Add Thread Sanitizer to build and test workflow [\#692](https://github.com/eclipse-iceoryx/iceoryx/issues/692)

**New API features:**

**API Breaking Changes:**

1. Builder pattern in `SharedMemoryObject` instead of creation pattern

    ```cpp
    // before
    auto sharedMemory = iox::posix::SharedMemoryObject::create("shmAllocate",
                                                      16,
                                                      iox::posix::AccessMode::READ_WRITE,
                                                      iox::posix::OpenMode::PURGE_AND_CREATE,
                                                      iox::posix::SharedMemoryObject::NO_ADDRESS_HINT);

    // after
    auto sharedMemory = iox::posix::SharedMemoryObjectBuilder()
                            .name("shmAllocate")
                            .memorySizeInBytes(16)
                            .accessMode(iox::posix::AccessMode::READ_WRITE)
                            .openMode(iox::posix::OpenMode::PURGE_AND_CREATE)
                            .permissions(iox::perms::owner_all)
                            .create();
    ```

2. Builder pattern extracted from `helplets.hpp` into `iox/builder.hpp`

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/helplets.hpp"

    // after
    #include "iox/builder.hpp"
    ```

3. `UnnamedSemaphore` replaces `Semaphore` with `CreateUnnamed*` option

    ```cpp
    // before
    #include "iceoryx_hoofs/posix_wrapper/semaphore.hpp"

    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0);

    // after
    #include "iceoryx_hoofs/posix_wrapper/unnamed_semaphore.hpp"

    iox::optional<iox::posix::UnnamedSemaphore> semaphore;
    auto result = iox::posix::UnnamedSemaphoreBuilder()
                    .initialValue(0U)
                    .isInterProcessCapable(true)
                    .create(semaphore);
    ```

4. `NamedSemaphore` replaces `Semaphore` with `CreateNamedSemaphore` option

    ```cpp
    // before
    #include "iceoryx_hoofs/posix_wrapper/semaphore.hpp"

    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateNamedSemaphore,
                                               "mySemaphoreName",
                                               S_IRUSR | S_IWUSR,
                                                   0);
    // after
    #include "iceoryx_hoofs/posix_wrapper/named_semaphore.hpp"

    iox::optional<iox::posix::NamedSemaphore> semaphore;
    auto result = iox::posix::NamedSemaphoreBuilder()
                    .name("mySemaphoreName")
                    .openMode(iox::posix::OpenMode::OPEN_OR_CREATE)
                    .permissions(iox::perms::owner_all)
                    .initialValue(0U)
                    .create(semaphore);
    ```

5. `RoudiApp::waitForSignal` is deprecated

    ```cpp
    // before
    //// in my custom roudi app implementation
    uint8_t MyCustomRoudiApp::run() noexcept {
        // ...

        waitForSignal();
    }

    // after
    //// in my custom roudi app implementation
    uint8_t MyCustomRoudiApp::run() noexcept {
        // ...

        iox::posix::waitForTerminationRequest();
    }
    ```

6. It is not possible to delete a class which is derived from `FunctionalInterface`
   via a pointer to `FunctionalInterface`

   ```cpp
   iox::FunctionalInterface<iox::optional<MyClass>, MyClass, void>* soSmart =
       new iox::optional<MyClass>{};

   delete soSmart; // <- not possible anymore
   ```

7. It is not possible to delete a class which is derived from `NewType` via a pointer to `NewType`

   ```cpp
   struct Foo : public iox::NewType<uint64_t, iox::newtype::ConstructByValueCopy>
   {
       using ThisType::ThisType;
   };

   iox::NewType<uint64_t, iox::newtype::ConstructByValueCopy>* soSmart = new Foo{42};

   delete soSmart; // <- not possible anymore
   ```

8. It is not possible to use the `NewType` to create type aliases. This was not recommended and is now enforced

   ```cpp
   // before
   // for the compiler Foo and Bar are the same type
   using Foo = iox::NewType<uint64_t, iox::newtype::ConstructByValueCopy>;
   using Bar = iox::NewType<uint64_t, iox::newtype::ConstructByValueCopy>;

   // after
   // compile time error when Foo and Bar are mixed up
   struct Foo : public iox::NewType<uint64_t, iox::newtype::ConstructByValueCopy>
   {
       using ThisType::ThisType;
   };
   // or with the IOX_NEW_TYPE macro
   IOX_NEW_TYPE(Bar, uint64_t, iox::newtype::ConstructByValueCopy);
   ```

9. `FileLock` uses the builder pattern. Path and permissions can now be set.

    ```cpp
    // before
    auto fileLock = iox::posix::FileLock::create("lockFileName")
                        .expect("Oh no I couldn't create the lock file");

    // after
    auto fileLock = iox::posix::FileLockBuilder().name("lockFileName")
                                                 .path("/Now/I/Can/Add/A/Path")
                                                 .permission(iox::perms::owner_all)
                                                 .create()
                                                 .expect("Oh no I couldn't create the lock file");
    ```

10. `isValidFilePath` is removed use `isValidPathToFile` instead.

    ```cpp
    // before
    bool isCorrect = isValidFilePath("/path/to/file");

    // after
    bool isCorrect = isValidPathToFile("/path/to/file");
    ```

11. Remove implicit conversion from `iox::expected` to `iox::optional`

    ```cpp
    // before
    iox::optional<int> myLama = someExpected;

    // after
    iox::optional<int> myLama = someExpected.to_optional();
    ```

12. Replace implicit conversion of `units::Duration` to `timeval` by a conversion method

    ```cpp
    // before
    units::Duration duration = 42_ms;
    timeval tv1 = duration;

    // after
    units::Duration duration = 42_ms;
    timveal tv = duration.timeval();
    ```

13. `registerSignalHandler` returns guard packed inside expected

    ```cpp
    // before
    //// unable to determine if an error occurred in the underlying posix calls
    auto signalGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);

    // after
    auto signalGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    if (signalGuard.has_error()) {
        // perform error handling
    }
    ```

14. Moved or removed various functions from helplets

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/helplets.hpp"
    iox::cxx::forEach(container, [&] (element) { /* do stuff with element */ });

    // after
    for (const auto& element: container) { /* do stuff with element */ }
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/helplets.hpp"
    iox::cxx::greater_or_equal(..);
    iox::cxx::range(..);
    iox::cxx::BestFittingType(..);
    iox::cxx::isPowerOfTwo(..);

    // after
    #include "iox/algorithm.hpp"
    iox::greater_or_equal(..);
    iox::range(..);
    iox::BestFittingType(..);
    iox::isPowerOfTwo(..);
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/helplets.hpp"
    iox::cxx::align(..);
    iox::cxx::alignedAlloc(..);
    iox::cxx::alignedFree(..);
    iox::cxx::maxAlignment(..);
    iox::cxx::maxSize(..);

    // after
    #include "iox/memory.hpp"
    iox::align(..);
    iox::alignedAlloc(..);
    iox::alignedFree(..);
    iox::maxAlignment(..);
    iox::maxSize(..);
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/helplets.hpp"
    iox::cxx::isValidPathEntry(..);
    iox::cxx::isValidFileName(..);
    iox::cxx::isValidPathToFile(..);
    iox::cxx::isValidPathToDirectory(..);
    iox::cxx::doesEndWithPathSeparator(..);

    // after
    #include "iox/filesystem.hpp"
    iox::isValidPathEntry(..);
    iox::isValidFileName(..);
    iox::isValidPathToFile(..);
    iox::isValidPathToDirectory(..);
    iox::doesEndWithPathSeparator(..);
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/helplets.hpp"
    template <>
    constexpr DestType
    iox::cxx::from<SourceType, DestType>(const SourceType value);
    iox::cxx::into(..);

    // after
    #include "iox/into.hpp"
    template <>
    constexpr DestType
    iox::from<SourceType, DestType>(const SourceType value);
    iox::into(..);
    ```

15. Remove `enumTypeAsUnderlyingType`

    ```cpp
    constexpr const char* SOME_ENUM_STRINGS[] = {"FOO", "BAR"};

    // before
    std::cout << SOME_ENUM_STRINGS[iox::cxx::enumTypeAsUnderlyingType(someEnum)] << std::endl;

    // after
    std::cout << SOME_ENUM_STRINGS[static_cast<uint64_t>(someEnum)] << std::endl;
    ```

16. Remove `convertEnumToString`

    ```cpp
    constexpr const char* SOME_ENUM_STRINGS[] = {"FOO", "BAR"};

    // before
    std::cout << iox::cxx::convertEnumToString(SOME_ENUM_STRINGS, someEnum] << std::endl;

    // after
    std::cout << SOME_ENUM_STRINGS[static_cast<uint64_t>(someEnum)] << std::endl;
    ```

17. Replace `strlen2` with more generic `iox::size`

    ```cpp
    constexpr const char LITERAL1[] {"FOO"};
    constexpr const char LITERAL2[20] {"BAR"};
    constexpr const uint32_t ARRAY[42] {};

    // before
    std::cout << iox::cxx::strlen2(LITERAL1) << std::endl; // prints 3
    std::cout << iox::cxx::strlen2(LITERAL2) << std::endl; // prints 19

    // after
    #include "iox/size.hpp"
    std::cout << iox::size(LITERAL1) << std::endl; // prints 4
    std::cout << iox::size(LITERAL2) << std::endl; // prints 20
    std::cout << iox::size(ARRAY) << std::endl;    // prints 42
    ```

18. Rename `cxx::GenericRAII` to `iox::ScopeGuard`

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/generic_raii.hpp"
    iox::cxx::GenericRAII {[]()
    {
        // do on creation
    },[]()
    {
        // do on destruction
    }};

    // after
    #include "iox/scope_guard.hpp"
    iox::ScopeGuard {[]()
    {
        // do on creation
    },[]()
    {
        // do on destruction
    }};
    ```

19. Rename `algorithm::max` and `algorithm::min` to `algorithm::maxVal` and `algorithm::minVal`

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/algorithm.hpp"
    constexpr uint32_t MAX_VAL = algorithm::max(3, 1890, 57);
    constexpr uint32_t MIN_VAL = algorithm::min(3, 1890, 57);

    // after
    #include "iox/algorithm.hpp"
    constexpr uint32_t MAX_VAL = algorithm::maxVal(3, 1890, 57);
    constexpr uint32_t MIN_VAL = algorithm::minVal(3, 1890, 57);
    ```

20. `ReleativePointer::registerPtr` returns `iox::optional`

    ```cpp
    // before
    uint64_t id = RelativePointer::register(startAddress, numBytes);

    if(id == INVALID_ID)
    {
        // Early exit
    }

    // after
    auto maybeId = RelativePointer::register(startAddress, numBytes);

    if(!id.has_value())
    {
        // Early exit
    }
    ```

21. Renamed `BaseRelativePointer` to `UntypedRelativePointer` and removed it from namespace `rp::`

    ```cpp
    // before
    #include "iceoryx_hoofs/internal/relocatable_pointer/base_relative_pointer.hpp"
    iox::rp::BaseRelativePointer myUntypedRelativePointer;

    // after
    #include "iox/relative_pointer.hpp"
    iox::UntypedRelativePointer myUntypedRelativePointer;
    ```

22. The `CMakeLists.txt` of apps using iceoryx need to add `iceoryx_platform`

    ```cmake
    # before
    cmake_minimum_required(VERSION 3.16)
    project(example)
    find_package(iceoryx_posh CONFIG REQUIRED)
    find_package(iceoryx_hoofs CONFIG REQUIRED)

    get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD) // obsolete

    include(IceoryxPackageHelper)
    include(IceoryxPlatform)

    # after
    cmake_minimum_required(VERSION 3.16)
    project(example)
    find_package(iceoryx_platform REQUIRED)         # new
    find_package(iceoryx_posh CONFIG REQUIRED)
    find_package(iceoryx_hoofs CONFIG REQUIRED)

    include(IceoryxPackageHelper)
    include(IceoryxPlatform)
    include(IceoryxPlatformSettings)                # new
    ```

23. `iceoryx_hoofs/platform` was moved into separate package `iceoryx_platform`. All includes must
    be adjusted.

    ```cpp
    // before
    #include "iceoryx_hoofs/platform/some_header.hpp"

    // after
    #include "iceoryx_platform/some_header.hpp"
    ```

24. `cxx::unique_ptr` is no longer nullable and does not have a `reset` method anymore

    ```cpp
    // before
    cxx::unique_ptr<int> myPtr(ptrToInt, someDeleter);
    cxx::unique_ptr<int> emptyPtr(nullptr, someDeleter);

    if (myPtr) { // required since the object could always be null
        std::cout << *myPtr << std::endl;
    }

    myPtr.reset(ptrToOtherInt);
    myPtr.release();


    // after
    iox::unique_ptr<int> myPtr(ptrToInt, someDeleter);
    iox::optional<cxx::unique_ptr<int>> emptyPtr(nullopt); // if unique_ptr shall be nullable use optional

    // no more null check required since it is no longer nullable
    std::cout << *myPtr << std::endl;

    myPtr = std::move(uniquePtrToAnotherInt); // deleter(myPtr) is called before move
    iox::unique_ptr<int>::release(std::move(myPtr)); // release consumes myPtr
    ```

    Compilers like ``gcc-12>`` and `clang>14` as well as static code analysis tools like `clang-tidy`
    will warn the user with a used after move warning when one accesses a moved object. Accessing
    a moved `unique_ptr` is well defined and behaves like dereferencing a `nullptr`.

25. `mutex` must be always stored inside an `iox::optional` and must use the builder pattern for
    construction

    ```cpp
    // before
    bool isRecursiveMutex = true;
    mutex myMutex(isRecursiveMutex);
    myMutex.lock();

    // after
    iox::optional<mutex> myMutex;
    iox::posix::MutexBuilder()
        .mutexType(iox::posix::MutexType::RECURSIVE)
        .create(myMutex);
    myMutex->lock();
    ```

26. Change return type of `vector::erase` from iterator to bool

    ```cpp
    // before
    auto* iter = myCxxVector.erase(myCxxVector.begin());

    // after
    bool success = myCxxVector.erase(myCxxVector.begin());
    ```

27. `iox::function` is no longer nullable.

    ```cpp
    // before
    iox::cxx::function<void()> helloFunc = []{ std::cout << "hello world\n"; };
    iox::cxx::function<void()> emptyFunction;

    if (helloFunc) { // required since the object could always be null
        helloFunc();
    }

    // after
    iox::function<void()> helloFunc = []{ std::cout << "hello world\n"; };
    iox::optional<iox::function<void()>> emptyPtr(nullopt); // if function shall be nullable use optional

    // no more null check required since it is no longer nullable
    helloFunc();
    ```

    Compilers like ``gcc-12>`` and `clang>14` as well as static code analysis tools like `clang-tidy`
    will warn the user with a used after move warning when one accesses a moved object. Accessing
    a moved `function` is well defined and behaves like dereferencing a `nullptr`.

28. `LogLevel` enum tags are renamed to better match the log4j log levels

    | before     | after   |
    |:----------:|:-------:|
    | `kOff`     | `OFF`   |
    | `kFatal`   | `FATAL` |
    | `kError`   | `ERROR` |
    | `kWarn`    | `WARN`  |
    | `kInfo`    | `INFO`  |
    | `kDebug`   | `DEBUG` |
    | `kVerbose` | `TRACE` |

    In the C binding the `Iceoryx_LogLevel_Verbose` changed to `Iceoryx_LogLevel_Trace`.

29. `LogLevel` enum moved from `iceoryx_hoofs/log/logcommon.hpp` to `iox/iceoryx_hoofs_types.hpp`

30. Using multiple logger instances and logging directly via a logger instance in not supported anymore out of the box

    ```cpp
    // before
    #include "iceoryx_hoofs/log/logmanager.hpp"

    auto& logger = iox::log::createLogger("MyComponent", "MyContext", iox::log::LogLevel::kInfo);

    logger.LogInfo() << "Hello World";

    // after
    #include "iox/logging.hpp"

    iox::log::Logger::init(iox::log::LogLevel::INFO);

    IOX_LOG(INFO) << "Hello World";
    ```

31. Setting the default log level changed

    ```cpp
    // before
    #include "iceoryx_hoofs/log/logmanager.hpp"

    iox::log::LogManager::GetLogManager().SetDefaultLogLevel(iox::log::LogLevel::kError);

    // after
    #include "iox/logging.hpp"

    iox::log::Logger::init(iox::log::LogLevel::ERROR);
    ```

    Please look at the logger design document for more details like setting the log level via environment variables.

32. Changing the log level at runtime changed

    ```cpp
    // before
    logger.SetLogLevel(); // directly on the instance

    // after
    iox::log::Logger::setLogLevel(iox::log::LogLevel::DEBUG);
    ```

33. Using the logger in libraries is massively simplified

    ```cpp
    // before
    // ==== file foo_logging.hpp ====
    #ifndef FOO_LOGGING_HPP_INCLUDED
    #define FOO_LOGGING_HPP_INCLUDED

    #include "iceoryx_hoofs/log/logging_free_function_building_block.hpp"

    namespace foo
    {
        struct LoggingComponent
        {
            static constexpr char Ctx[] = "FOO";
            static constexpr char Description[] = "Log context of the FOO component!";
        };

        static constexpr auto LogFatal = iox::log::ffbb::LogFatal<LoggingComponent>;
        static constexpr auto LogError = iox::log::ffbb::LogError<LoggingComponent>;
        static constexpr auto LogWarn = iox::log::ffbb::LogWarn<LoggingComponent>;
        static constexpr auto LogInfo = iox::log::ffbb::LogInfo<LoggingComponent>;
        static constexpr auto LogDebug = iox::log::ffbb::LogDebug<LoggingComponent>;
        static constexpr auto LogVerbose = iox::log::ffbb::LogVerbose<LoggingComponent>;
    } // namespace foo
    #endif // FOO_LOGGING_HPP_INCLUDED

    // ==== file foo_logging.cpp ====
    #include "foo_logging.hpp"

    namespace foo
    {
        constexpr char ComponentPosh::Ctx[];
        constexpr char ComponentPosh::Description[];

    } // namespace foo

    // ==== file bar.cpp ====
    #include "foo_logging.hpp"

    namespace foo
    {
        void myFunc()
        {
            LogInfo() << "Hello World";
        }
    }


    // after
    // ==== file bar.cpp ====
    #include "iox/logging.hpp"

    namespace foo
    {
        void myFunc()
        {
            IOX_LOG(INFO) << "Hello World";
        }
    }
    ```

34. Free function logger calls changed

    | before         | after            |
    |:--------------:|:----------------:|
    | `LogFatal()`   | `IOX_LOG(FATAL)` |
    | `LogError()`   | `IOX_LOG(ERROR)` |
    | `LogWarn()`    | `IOX_LOG(WARN)`  |
    | `LogInfo()`    | `IOX_LOG(INFO)`  |
    | `LogDebug()`   | `IOX_LOG(DEBUG)` |
    | `LogVerbose()` | `IOX_LOG(TRACE)` |

35. Logger formatting changed

    ```cpp
    // before
    LogInfo() << iox::log::HexFormat(42);
    LogInfo() << iox::log::BinFormat(73); // currently not supported
    LogInfo() << iox::log::RawBuffer(buf); // currently not supported

    // after
    IOX_LOG(INFO) << iox::log::hex(42);
    IOX_LOG(INFO) << iox::log::oct(42);
    ```

36. Creating an instance of `LogStream` does not work anymore

    ```cpp
    // before
    auto stream = LogInfo();
    stream << "fibonacci: "
    for(auto fib : {1, 1, 2, 3, 5, 8})
    {
        stream << fib << ", ";
    }
    stream << "...";
    stream.Flush();

    // after
    IOX_LOG(INFO) << [] (auto& stream) -> auto& {
        stream << "fibonacci: "
        for(auto fib : {1, 1, 2, 3, 5, 8})
        {
            stream << fib << ", ";
        }
        stream << "...";
        return stream;
    };
    ```

37. Testing of `LogStream::operator<<` overload for custom types changed

    ```cpp
    // before
    Logger_Mock loggerMock;
    iox::log::LogStream(loggerMock) << myType;

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.m_logs[0].message, StrEq(EXPECTED_STRING));

    // after
    iox::testing::Logger_Mock loggerMock;
    IOX_LOGSTREAM_MOCK(loggerMock) << myType;

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq(EXPECTED_STRING));
    ```

38. Suppressing the logger output in tests

    ```cpp
    // before
    // using gTest ::testing::internal::CaptureStdout() or ::testing::internal::CaptureStderr()
    // in every test fixture setup method. This also suppresses the output of the sanitizer
    // and makes debugging of CI failures unnecessary hard. In addition, it might crash the unittest
    // when `EXPECT_DEATH` is used

    // after
    // ==== unittests.cpp ====
    #include "iceoryx_hoofs/testing/testing_logger.hpp"

    #include <gtest/gtest.h>

    int main(int argc, char* argv[])
    {
        ::testing::InitGoogleTest(&argc, argv);

        iox::testing::TestingLogger::init();

        return RUN_ALL_TESTS();
    }
    ```

    The log messages are cached and printed when a test fails. To print log messages also for passed tests,
    the `IOX_TESTING_ALLOW_LOG` environment variable can be used,
    e.g. `IOX_TESTING_ALLOW_LOG=ON ./unittests --gtest_filter=MyTest\*`. This might be helpful to debug tests.

39. Checking the log message of test objects in unit tests

    ```cpp
    // before
    // some wild stuff with std::clog redirecting or ::testing::internal::CaptureStdout()
    sut.methodCallWithLogOutput();
    // some wild stuff getting the output from the redirected clog or ::testing::internal::internal::GetCapturedStdout()

    // after
    #include "iceoryx_hoofs/testing/testing_logger.hpp"

    sut.methodCallWithLogOutput();
    if (iox::testing::TestingLogger::doesLoggerSupportLogLevel(iox::log::LogLevel::ERROR))
    {
        auto logMessages = iox::testing::TestingLogger::getLogMessages();
        ASSERT_THAT(logMessages.size(), Eq(1U));
        EXPECT_THAT(logMessages[0], HasSubstr(expectedOutput));
    }
    ```

    Have a look at the logger design document for more details on how to setup the testing logger.

40. Changed the include path and namespace of several classes in `iceoryx_hoofs`:

    * `iox::bar::foo` to `iox::foo`
        * `iceoryx_hoofs/bar/foo.hpp` to `iox/foo.hpp`

41. Use proper aligned `iox::UninitializedArray` instead of C-style array

    ```cpp
    // before
    char myCharArray[Capacity];

    using element_t = uint8_t[sizeof(T)];
    alignas(T) element_t myAlignedArray[Capacity];

    // after
    #include "iox/uninitialized_array.hpp"

    iox::UninitializedArray<char, Capacity, iox::ZeroedBuffer> myCharArray;

    iox::UninitializedArray<T, Capacity> myAlignedArray;

    ```

42. Move multiple classes from `iceoryx_hoofs` to `iceoryx_dust`

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/forward_list.hpp"

    #include "iceoryx_dust/cxx/forward_list.hpp"
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/design_pattern/creation.hpp"

    // after
    #include "iceoryx_dust/design/creation.hpp"
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/internal/cxx/static_storage.hpp"

    // after
    #include "iceoryx_dust/internal/cxx/static_storage.hpp"
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/internal/file_reader/file_reader.hpp"

    // after
    #include "iceoryx_dust/cxx/file_reader.hpp"
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/internal/objectpool/objectpool.hpp"

    // after
    #include "iceoryx_dust/cxx/objectpool.hpp"
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/internal/relocatable_pointer/relocatable_ptr.hpp"

    // after
    #include "iceoryx_dust/relocatable_pointer/relocatable_ptr.hpp"
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/posix_wrapper/internal/message_queue.hpp"

    // after
    #include "iceoryx_dust/posix_wrapper/message_queue.hpp"
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/posix_wrapper/named_pipe.hpp"

    // after
    #include "iceoryx_dust/posix_wrapper/named_pipe.hpp"
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"

    // after
    #include "iceoryx_dust/posix_wrapper/signal_watcher.hpp"
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/serialization.hpp"

    // after
    #include "iceoryx_dust/cxx/serialization.hpp"
    ```

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/convert.hpp"

    // after
    #include "iceoryx_dust/cxx/convert.hpp"
    ```

43. Move the conversions functions for `std::string` to `iceoryx_dust`:

    ```cpp
    // before
    std::string myStdString("foo");
    // std::string to iox::string
    iox::string<3> myIoxString(TruncateToCapacity, myStdString);
    // iox::string to std::string
    std::string myConvertedIoxString = static_cast<std::string>(myIoxString);

    // after
    #include "iceoryx_dust/cxx/std_string_support.hpp"

    std::string myStdString("foo");
    // std::string to iox::string with truncation when source string exceeds capacity
    auto myIoxString = iox::into<iox::lossy<iox::string<3>>>(myStdString); // returns a 'iox::string<3>'
    // std::string to iox::string with fallible conversion when source string exceeds capacity
    auto maybeMyIoxString = iox::into<iox::optional<iox::string<3>>>(myStdString); // returns a 'iox::optional<iox::string<3>>'
    // iox::string to std::string
    auto myConvertedIoxString = iox::into<std::string>(myIoxString); // returns a 'std::string'
    ```

44. In order to use the comparison or `operator<<` operators, `insert`, `find`, `unsafe_append` functions for
    `std::string` together with `iox::string` include the support header:

    ```cpp
    // before
    std::string myStdString("foo");
    iox::string<3> myIoxString("foo");
    if(myIoxString == myStdString)
    {
      ..
    }

    // after
    #include "iceoryx_dust/cxx/std_string_support.hpp"

    std::string myStdString("foo");
    iox::string<3> myIoxString("foo");
    if(myIoxString == myStdString)
    {
      ..
    }
    ```

45. Move and rename `DeadlineTimer`

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/deadline_timer.hpp"
    iox::cxx::DeadlineTimer myTimer;

    // after
    #include "iox/deadline_timer.hpp"
    iox::deadline_timer myTimer;
    ```

46. Changed include path of `iox::units::Duration`

    ```cpp
    // before
    #include "iceoryx_hoofs/internal/units/duration.hpp"

    // after
    #include "iox/duration.hpp"
    ```

47. The `perms` enum is replaced by the `access_rights` class

    ```cpp
    // before
    iox::perms foo { iox::perms::owner_all | iox::perms::group_read };

    // after
    iox::access_rights foo { iox::perms::owner_all | iox::perms::group_read };
    ```

47. Renaming `byte_t` to `byte`

    ```cpp
    // before
    iox::byte_t m_size;

    // after
    iox::byte m_size;
    ```

48. Move conversion methods from `duration.hpp` to `iceoryx_dust`

    ```cpp
    // before
    std::chrono::milliseconds chronoDuration = 1_ms;
    iox::units::Duration ioxDuration(chronoDuration);

    // after
    #include "iceoryx_dust/cxx/std_chrono_support.hpp"

    std::chrono::milliseconds chronoDuration = 1_ms;
    iox::units::Duration ioxDuration{into<iox::units::Duration>(chronoDuration)};
    ```

49. Replace error only `expected<E>` with `void` value type `expected<void, E>`

    ```cpp
    // before
    iox::expected<MyCustomError> foo();

    // after
    iox::expected<void, MyCustomError> foo();
    ```

50. `iox::success` and `iox::error` are deprecated in favour of `ok` and `err` free functions

    ```cpp
    // before
    return iox::success<void>();

    // after
    return iox::ok();


    // before
    return iox::success<bool>(true);

    // after
    return iox::ok(true);


    // before
    return iox::error<MyCustomError>(MyCustomError::ERROR_CODE);

    // after
    return iox::err(MyCustomError::ERROR_CODE);
    ```

51. `expected::get_error` is deprecated in favour of `expected::error`

    ```cpp
    // before
    auto e = exp.get_error();

    // after
    auto e = exp.error();
