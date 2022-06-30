# iceoryx vx.x.x

## [vx.x.x](https://github.com/eclipse-iceoryx/iceoryx/tree/vx.x.x) (xxxx-xx-xx) <!--NOLINT remove this when tag is set-->

[Full Changelog](https://github.com/eclipse-iceoryx/iceoryx/compare/vx.x.x...vx.x.x) <!--NOLINT remove this when tag is set-->

**Features:**

- optional inherits from FunctionalInterface, adds .expect() method [\#996](https://github.com/eclipse-iceoryx/iceoryx/issues/996)
- Add clear method for `iox::cxx::string` [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- Add at method and operator[] for `iox::cxx::string` [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- expected inherits from FunctionalInterface, adds .expect() method [\#996](https://github.com/eclipse-iceoryx/iceoryx/issues/996)
- Added CI check of used headers against a list [\#1252](https://github.com/eclipse-iceoryx/iceoryx/issues/1252)
- Add insert method for `iox::cxx::string` [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- Extend compare method of `iox::cxx::string` to compare additionally with std::string and char array [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- Add compare method for `iox::cxx::string` and chars [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- Refactor semaphore [\#751](https://github.com/eclipse-iceoryx/iceoryx/issues/751)
    - Introduce `UnnamedSemaphore`
    - Introduce `NamedSemaphore`
    - Remove old `Semaphore`
- Extend `concatenate`, `operator+`, `unsafe_append` and `append` of `iox::cxx::string` for chars [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- Extend `unsafe_append` and `append` methods of `iox::cxx::string` for `std::string` [\#208](https://github.com/eclipse-iceoryx/iceoryx/issues/208)
- The iceoryx development environment supports multiple running docker containers [\#1410](https://github.com/eclipse-iceoryx/iceoryx/issues/1410)
- Use builder pattern in FileLock [\#1036](https://github.com/eclipse-iceoryx/iceoryx/issues/1036)
    - Add the ability to adjust path and file permissions of the file lock
- Create convenience macro for `NewType` [\#1425](https://github.com/eclipse-iceoryx/iceoryx/issues/1425)
- Add posix thread wrapper [\#1365](https://github.com/eclipse-iceoryx/iceoryx/issues/1365)

**Bugfixes:**

- FreeBSD CI build is broken [\#1338](https://github.com/eclipse-iceoryx/iceoryx/issues/1338)
- High CPU load in blocked publisher is reduced by introducing smart busy loop waiting (adaptive_wait) [\#1347](https://github.com/eclipse-iceoryx/iceoryx/issues/1347)
- Compile Error : iceoryx_dds/Mempool.hpp: No such file or directory [\#1364](https://github.com/eclipse-iceoryx/iceoryx/issues/1364)
- RPATH is correctly set up for all libraries and binaries. [\#1287](https://github.com/eclipse-iceoryx/iceoryx/issues/1287)
- Wrong memory order in concurrent::FIFO [\#1396](https://github.com/eclipse-iceoryx/iceoryx/issues/1396)
- Iceoryx libraries weren't compiled with `-fPIC` as position independent code [#\879](https://github.com/eclipse-iceoryx/iceoryx/issues/879)
- Restrict runtime (application) names to valid file names to solve failures in the underlying posix constructs [#\1419](https://github.com/eclipse-iceoryx/iceoryx/issues/1419)

**Refactoring:**

- Separate module specific errors from `iceoryx_hoofs` [\#1099](https://github.com/eclipse-iceoryx/iceoryx/issues/1099)
  - Move test specific code to `ErrorHandlerMock` and templatize `setTemporaryErrorHandler()`
  - Create separate error enum for each module
- Use `GTEST_FAIL` and `GTEST_SUCCEED` instead of `FAIL` and `SUCCEED` [\#1072](https://github.com/eclipse-iceoryx/iceoryx/issues/1072)
- posix wrapper `SharedMemoryObject` is silent on success [\#971](https://github.com/eclipse-iceoryx/iceoryx/issues/971)
- Remove creation design pattern class with in place implementation [\#1036](https://github.com/eclipse-iceoryx/iceoryx/issues/1036)
  - posix wrapper `SharedMemoryObject` uses builder pattern instead of creation
  - Builder pattern extracted from `helplets.hpp` into `design_pattern/builder.hpp`
- Uninteresting mock function calls in tests [\#1341](https://github.com/eclipse-iceoryx/iceoryx/issues/1341)
- `cxx::unique_ptr` owns deleter, remove all deleter classes [\#1143](https://github.com/eclipse-iceoryx/iceoryx/issues/1143)
- Remove `iox::posix::Timer` [\#337](https://github.com/eclipse-iceoryx/iceoryx/issues/337)
- Refactor service discovery tests [/#1065](https://github.com/eclipse-iceoryx/iceoryx/issues/1065)
  to increase comprehension and cover more test cases
- Remove usage of `std::function` [\#831](https://github.com/eclipse-iceoryx/iceoryx/issues/831)
- Replace `MethodCallback` with `cxx::function` [\#831](https://github.com/eclipse-iceoryx/iceoryx/issues/831)

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
                            .permissions(cxx::perms::owner_all)
                            .create();
    ```

2. Builder pattern extracted from `helplets.hpp` into `design_pattern/builder.hpp`

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/helplets.hpp"

    // after
    #include "iceoryx_hoofs/design_pattern/builder.hpp"
    ```

3. `UnnamedSemaphore` replaces `Semaphore` with `CreateUnnamed*` option

    ```cpp
    // before
    #include "iceoryx_hoofs/posix_wrapper/semaphore.hpp"

    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0);

    // after
    #include "iceoryx_hoofs/posix_wrapper/unnamed_semaphore.hpp"

    iox::cxx::optional<iox::posix::UnnamedSemaphore> semaphore;
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

    iox::cxx::optional<iox::posix::NamedSemaphore> semaphore;
    auto result = iox::posix::NamedSemaphoreBuilder()
                    .name("mySemaphoreName")
                    .openMode(iox::posix::OpenMode::OPEN_OR_CREATE)
                    .permissions(iox::cxx::perms::owner_all)
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
   iox::cxx::FunctionalInterface<iox::cxx::optional<MyClass>, MyClass, void>* soSmart =
       new iox::cxx::optional<MyClass>{};

   delete soSmart; // <- not possible anymore
   ```

7. It is not possible to delete a class which is derived from `NewType` via a pointer to `NewType`

   ```cpp
   struct Foo : public iox::cxx::NewType<uint64_t, iox::cxx::newtype::ConstructByValueCopy>
   {
       using ThisType::ThisType;
   };

   iox::cxx::NewType<uint64_t, iox::cxx::newtype::ConstructByValueCopy>* soSmart = new Foo{42};

   delete soSmart; // <- not possible anymore
   ```

8. It is not possible to use the `NewType` to create type aliases. This was not recommended and is now enforced

   ```cpp
   // before
   // for the compiler Foo and Bar are the same type
   using Foo = iox::cxx::NewType<uint64_t, iox::cxx::newtype::ConstructByValueCopy>;
   using Bar = iox::cxx::NewType<uint64_t, iox::cxx::newtype::ConstructByValueCopy>;

   // after
   // compile time error when Foo and Bar are mixed up
   struct Foo : public iox::cxx::NewType<uint64_t, iox::cxx::newtype::ConstructByValueCopy>
   {
       using ThisType::ThisType;
   };
   // or with the IOX_NEW_TYPE macro
   IOX_NEW_TYPE(Bar, uint64_t, iox::cxx::newtype::ConstructByValueCopy);
   ```

9. `FileLock` uses the builder pattern. Path and permissions can now be set.
    ```cpp
    // before
    auto fileLock = iox::posix::FileLock::create("lockFileName")
                        .expect("Oh no I couldn't create the lock file");

    // after
    auto fileLock = iox::posix::FileLockBuilder().name("lockFileName")
                                                 .path("/Now/I/Can/Add/A/Path")
                                                 .permission(iox::cxx::perms::owner_all)
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
