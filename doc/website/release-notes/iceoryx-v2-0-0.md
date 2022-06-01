# iceoryx v2.0.0

## [v2.0.0](https://github.com/eclipse-iceoryx/iceoryx/tree/v2.0.0) (2022-03-14)

[Full Changelog](https://github.com/eclipse-iceoryx/iceoryx/compare/v1.0.2...v2.0.0)

**Features:**

- block publisher when subscriber queue full [\#615](https://github.com/eclipse-iceoryx/iceoryx/issues/615)
- extend chunk header for gateways and recompute [\#711](https://github.com/eclipse-iceoryx/iceoryx/pull/711)
- Basic Windows 10 support [\#33](https://github.com/eclipse-iceoryx/iceoryx/issues/33)
- Common puml file for common settings [\#865](https://github.com/eclipse-iceoryx/iceoryx/issues/865)
- Relocatable Pointers - Version 2.0 [\#926](https://github.com/eclipse-iceoryx/iceoryx/issues/926)
- Implement `waitForTerminationRequest` [\#973](https://github.com/eclipse-iceoryx/iceoryx/issues/973)
- Partial enable iceoryx building with msvc2015+clang [\#965](https://github.com/eclipse-iceoryx/iceoryx/issues/965)
- C binding for posh configuration [\#930](https://github.com/eclipse-iceoryx/iceoryx/issues/930)
- Enhance MacOS performance with timed{send,receive} functionality in unix domain socket[\#903](https://github.com/eclipse-iceoryx/iceoryx/issues/903)
- Multi-Publisher support for DDS gateway and generic gateway class [\#900](https://github.com/eclipse-iceoryx/iceoryx/issues/900)
- Replace `iox-gw-iceoryx2dds` and `iox-gw-dds2iceoryx` gateways with `iox-dds-gateway` [\#900](https://github.com/eclipse-iceoryx/iceoryx/issues/900)
- Enhance `posixCall` [\#805](https://github.com/eclipse-iceoryx/iceoryx/issues/805)
- Git Hooks on iceoryx [\#486](https://github.com/eclipse-iceoryx/iceoryx/issues/486)
- static memory alternative for std::function [\#391](https://github.com/eclipse-iceoryx/iceoryx/issues/391)
- Adding support for Helix QAC 2021.1 [\#755](https://github.com/eclipse-iceoryx/iceoryx/issues/755) thanks to @toniglandy1
- Axivion analysis on CI [\#409](https://github.com/eclipse-iceoryx/iceoryx/issues/409)
- Cpptoml can be provided by an external source [\#951](https://github.com/eclipse-iceoryx/iceoryx/issues/)
- Extend `cxx::optional` constructor for in place construction so that copy/move for values inside the optional even could be deleted [\#967](https://github.com/eclipse-iceoryx/iceoryx/issues/967)
- Add templated `from`/`into` free functions to formalize conversions from enums and other types [#992](https://github.com/eclipse-iceoryx/iceoryx/issues/992)
- `UniqueId` class for unique IDs within a process [#1010](https://github.com/eclipse-iceoryx/iceoryx/issues/1010)
- Add `requirePublisherHistorySupport` option at subscriber side (if set to true requires historyCapacity > 0 to be eligible for connection) [#1029](https://github.com/eclipse-iceoryx/iceoryx/issues/1029), [#1278](https://github.com/eclipse-iceoryx/iceoryx/issues/1278)
- Add `/tools/scripts/ice_env.sh` shell script to provide simple access to docker containers for CI debugging [#1049](https://github.com/eclipse-iceoryx/iceoryx/issues/1049)
- Introduce `cxx::FunctionalInterface` to enrich nullable classes with `and_then`, `or_else`, `value_or`, `expect` [\#996](https://github.com/eclipse-iceoryx/iceoryx/issues/996)
- Add C++17 `std::perms` as `cxx::perms` to `iceoryx_hoofs/cxx/filesystem.hpp`. [#1059](https://github.com/eclipse-iceoryx/iceoryx/issues/1059)
- Support FreeBSD as a representative for the UNIX platforms [#1054](https://github.com/eclipse-iceoryx/iceoryx/issues/1054)
- Add event parameter to `findService` method [#415](https://github.com/eclipse-iceoryx/iceoryx/issues/415)
- Implement stream operator for `ChunkReceiveResult` and `AllocationError` to be able to use it with ostream and LogStream [\#1062](https://github.com/eclipse-iceoryx/iceoryx/issues/1062)
- Replace IPC-channel-based `findService` with pub/sub-based on [#415](https://github.com/eclipse-iceoryx/iceoryx/issues/415)
- Add `findService` method to `ServiceDiscovery` which applies a callable to all matching services [\#1105](https://github.com/eclipse-iceoryx/iceoryx/pull/1105)
- Increase limits of `ServiceRegistry` to support the maximum number of publishers and servers that are configured in `iceoryx_posh_types.hpp` [\#1074](https://github.com/eclipse-iceoryx/iceoryx/issues/1074)
- C binding for service discovery [\#1142](https://github.com/eclipse-iceoryx/iceoryx/issues/1142)
- Introduce `iox::popo::MessagingPattern` to `findService` to allow separate searches for publishers (`MessagingPattern::PUB_SUB`) and
servers (`iox::popo::MessagingPattern::REQ_RES`) [\#27](https://github.com/eclipse-iceoryx/iceoryx/pull/1134)
- Request/Response communication with iceoryx [\#27](https://github.com/eclipse-iceoryx/iceoryx/issues/27)
    - For more details how this feature can be used please have a look at the `iceoryx_examples/request_response`
    - Limitations
        - The port introspection is not aware of the new `Client` and `Server` [\#1128](https://github.com/eclipse-iceoryx/iceoryx/issues/1128)
        - The DDS gateway is not aware of the new `Server` [\#1145](https://github.com/eclipse-iceoryx/iceoryx/issues/1145)
- Set `MAX_NUMBER_OF_NOTIFIERS` to 256 and prepare configuration via CMake[\#1144](https://github.com/eclipse-iceoryx/iceoryx/issues/1144)
- Reorganize code in publisher.hpp/.inl and subscriber.hpp/inl [\#1173](https://github.com/eclipse-iceoryx/iceoryx/issues/1173)
- Install headers to `include/iceoryx/vX.Y.Z` by default and add CMake option `MAKE_UNIQUE_INCLUDEDIR` to control the behavior [\#1194](https://github.com/eclipse-iceoryx/iceoryx/issues/1194)

**Bugfixes:**

- Analyse suppressed errors of AddressSanitizer, LeakSanitizer & UndefinedBehaviorSanitizer [\#423](https://github.com/eclipse-iceoryx/iceoryx/issues/423)
- CMake file duplicate option in build_options.cmake [\#709](https://github.com/eclipse-iceoryx/iceoryx/issues/709) thanks to @ZhenshengLee
- SharedChunk should internally store an absolute pointer [\#713](https://github.com/eclipse-iceoryx/iceoryx/issues/713)
- loanPreviousChunk is broken [\#729](https://github.com/eclipse-iceoryx/iceoryx/issues/729)
- Runtime Error in SubscriberImpl [\#714](https://github.com/eclipse-iceoryx/iceoryx/issues/714)
- Wrong Values of iox-cpp-subscriber and iox-cpp-publisher [\#781](https://github.com/eclipse-iceoryx/iceoryx/issues/781)
- CMake fails during googletest step with gcc 11.1.0 [\#798](https://github.com/eclipse-iceoryx/iceoryx/issues/798)
- NewType Copy-Assign raises compiler warning on GCC 8.4.0 [\#282](https://github.com/eclipse-iceoryx/iceoryx/issues/282)
- Apply noexcept all the things rule to posh, hoofs and binding_c [\#916](https://github.com/eclipse-iceoryx/iceoryx/pull/916)
- Doxygen shows wrong include paths [\#922](https://github.com/eclipse-iceoryx/iceoryx/pull/922)
- `find_package(iceoryx_posh)` fails [\#944](https://github.com/eclipse-iceoryx/iceoryx/pull/944) thanks to @ijnek
- iox-roudi report error when running in docker [\#946](https://github.com/eclipse-iceoryx/iceoryx/pull/946)
- compile error: duration.inl - duration literals are not found (Windows) [\#1078](https://github.com/eclipse-iceoryx/iceoryx/pull/1078)
- `cxx::string` initialization with nullptr leads to segfault [\#1108](https://github.com/eclipse-iceoryx/iceoryx/pull/1108)
- Fix support for libc++ on clang [\#905](https://github.com/eclipse-iceoryx/iceoryx/issues/905)
- Fix warnings for gcc-11.1 [\#838](https://github.com/eclipse-iceoryx/iceoryx/issues/838)
- Incremental builds with the build script are broken [\#821](https://github.com/eclipse-iceoryx/iceoryx/issues/821)
- Compile failed because of missing `<limits>` for GCC 11 [\#811](https://github.com/eclipse-iceoryx/iceoryx/issues/811) thanks to @homalozoa
- Unable to build cyclone dds `idlpp-cxx` [\#736](https://github.com/eclipse-iceoryx/iceoryx/issues/736)
- Fix format string issues with introspection client [\#960](https://github.com/eclipse-iceoryx/iceoryx/issues/960) thanks to @roehling
- Add support for Multi-Arch install destinations [\#961](https://github.com/eclipse-iceoryx/iceoryx/issues/961) thanks to @roehling
- Fix a few misspellings in log messages [\#962](https://github.com/eclipse-iceoryx/iceoryx/issues/962) thanks to @roehling
- Fix typos in goals/non-goals document [\#968](https://github.com/eclipse-iceoryx/iceoryx/issues/968) thanks to @
fb913bf0de288ba84fe98f7a23d35edfdb22381
- Catch deserialization errors for enums in publisher and subscriber options [\#989](https://github.com/eclipse-iceoryx/iceoryx/issues/989)
- Fix linker error on QNX [\#1013](https://github.com/eclipse-iceoryx/iceoryx/issues/1013)
- When posix mutex fails a correct error message is reported on the console [\#999](https://github.com/eclipse-iceoryx/iceoryx/issues/999)
- Only use `std::result_of` for C++14 to be able to use iceoryx in C++20 projects [\#1076](https://github.com/eclipse-iceoryx/iceoryx/issues/1076)
- Set stack size for windows in `singleprocess` example and posh tests [\#1082](https://github.com/eclipse-iceoryx/iceoryx/issues/1082)
- Roudi console timestamps are out of date [#1130](https://github.com/eclipse-iceoryx/iceoryx/issues/1130)
- Application can't create publisher repeatedly with previous one already destroyed [\#938](https://github.com/eclipse-iceoryx/iceoryx/issues/938)
- Prevent creation of `popo::Publisher`'s with internal `ServiceDescription` [\#1120](https://github.com/eclipse-iceoryx/iceoryx/issues/1120)
- RelativePointer is now type safe, i.e. can only be constructed from pointers with a valid convertion to the raw pointer [\#1121](https://github.com/eclipse-iceoryx/iceoryx/issues/1121)
- Clamping `historyRequest` to `queueCapacity` [\#1192](https://github.com/eclipse-iceoryx/iceoryx/issues/1192)
- C binding storage sizes do not match for multiple OS's and architectures [\#1218](https://github.com/eclipse-iceoryx/iceoryx/issues/1218)
- Update cyclone dds version used by gateway to support aarch64 [\#1223](https://github.com/eclipse-iceoryx/iceoryx/issues/1223)
- The file lock posix wrapper unlocks and removes a file correctly [\#1216](https://github.com/eclipse-iceoryx/iceoryx/issues/1216)
- Minor fixes for the examples [\#743](https://github.com/eclipse-iceoryx/iceoryx/issues/743)
- Fix race condition in Windows platform semaphore/mutex posix implementation [\#1271](https://github.com/eclipse-iceoryx/iceoryx/issues/1271)
- Fix race condition in Windows platform HandleTranslator [\#1264](https://github.com/eclipse-iceoryx/iceoryx/issues/1264)
- Fix race condition in Windows platform shared memory implementation [\#1269](https://github.com/eclipse-iceoryx/iceoryx/issues/1269)

**Refactoring:**

- implement Module-Tests for smart_lock [\#588](https://github.com/eclipse-iceoryx/iceoryx/issues/588)
- improve Helix QAC parsing coverage [\#759](https://github.com/eclipse-iceoryx/iceoryx/issues/759) thanks to @toniglandy1
- Write "ROS 2" with a space between "ROS" and "2" [\#762](https://github.com/eclipse-iceoryx/iceoryx/issues/762) thanks to @christophebedard
- Enforce unix line endings [\#794](https://github.com/eclipse-iceoryx/iceoryx/issues/794)
- Rename utils to hoofs [\#790](https://github.com/eclipse-iceoryx/iceoryx/pull/790)
- Cleanup MemoryProvider and MemoryBlock [\#842](https://github.com/eclipse-iceoryx/iceoryx/issues/842)
- Remove #define private public from all tests [\#529](https://github.com/eclipse-iceoryx/iceoryx/issues/529)
- Write example on how to use iceoryx in a docker environment [\#924](https://github.com/eclipse-iceoryx/iceoryx/issues/924)
- Allow cpptoml to be provided externally and not vendored by CMake [\#950](https://github.com/eclipse-iceoryx/iceoryx/issues/950) thanks to @photex
- Reworked iceoryx examples [\#482](https://github.com/eclipse-iceoryx/iceoryx/issues/482)
- Handle nullptr callbacks in waitset and listener [\#932](https://github.com/eclipse-iceoryx/iceoryx/issues/932)
- Add CI job that checks formatting with clang-format [\#887](https://github.com/eclipse-iceoryx/iceoryx/pull/887)
- Add clang-tidy rules for iceoryx_hoofs [\#889](https://github.com/eclipse-iceoryx/iceoryx/issues/889)
- Consolidate CI jobs in one workflow [\#891](https://github.com/eclipse-iceoryx/iceoryx/issues/891)
- Move all tests into an anonymous namespace [\#563](https://github.com/eclipse-iceoryx/iceoryx/issues/563)
- Refactor `smart_c` to use contract by design and expected [\#418](https://github.com/eclipse-iceoryx/iceoryx/issues/418)
- `PoshRuntime` Mock [\#449](https://github.com/eclipse-iceoryx/iceoryx/issues/449)
- Clean-up Doxygen for dds [\#583](https://github.com/eclipse-iceoryx/iceoryx/issues/583)
- Rename utils to hoofs [\#790](https://github.com/eclipse-iceoryx/iceoryx/pull/790)
- plantuml in design documentation [\#787](https://github.com/eclipse-iceoryx/iceoryx/pull/787)
- Refine quality levels [\#425](https://github.com/eclipse-iceoryx/iceoryx/issues/425)
- Clean-up std::terminate usage [\#261](https://github.com/eclipse-iceoryx/iceoryx/issues/261)
- Add Quality Declaration Document [\#910](https://github.com/eclipse-iceoryx/iceoryx/issues/910)
- Make `cxx::string::capacity` a `static` method [\#979](https://github.com/eclipse-iceoryx/iceoryx/issues/979)
- Restructure iceoryx tools [\#471](https://github.com/eclipse-iceoryx/iceoryx/issues/471)
- Use cxx::expected for MemoryManager::getChunk [\#954](https://github.com/eclipse-iceoryx/iceoryx/pull/991)
- Upgrade GTest/GMock to v1.10 [\#841](https://github.com/eclipse-iceoryx/iceoryx/issues/841)
- Remove the requirement for INVALID_STATE for the cxx::expected [\#987](https://github.com/eclipse-iceoryx/iceoryx/issues/987)
- Add unique test identifers [\#988](https://github.com/eclipse-iceoryx/iceoryx/issues/988)
- Remove `ApplicationPort` and `ApplicationPortData` classes [\#415](https://github.com/eclipse-iceoryx/iceoryx/issues/415)
- Remove creation pattern from `MemoryMap` and replace it with `MemoryMapBuilder` [\#1036](https://github.com/eclipse-iceoryx/iceoryx/issues/1036)
- Fix error handling of `TypedUniqueId` and refactor it to `UniquePortId` [\#861](https://github.com/eclipse-iceoryx/iceoryx/issues/861)
- Updating Codecov API and enforce CMake version 3.16 for building iceoryx [\#774](https://github.com/eclipse-iceoryx/iceoryx/issues/774) and [\#1031](https://github.com/eclipse-iceoryx/iceoryx/issues/1031)
- Remove `InvalidIdString` and `isValid()` from `ServiceDescription`, replace Wildcard string with `iox::cxx::nullopt` [\#415](https://github.com/eclipse-iceoryx/iceoryx/issues/415)
- Remove creation pattern from `SharedMemory` and replace it with `SharedMemoryBuilder` [\#1036](https://github.com/eclipse-iceoryx/iceoryx/issues/1036)
- Remove the leading slash requirement from the name of a shared memory in `SharedMemory` and `SharedMemoryObject` [\#439](https://github.com/eclipse-iceoryx/iceoryx/issues/439)

**New API features:**

1. Introduce `iceoryx_hoofs/cxx/filesystem.hpp` which implements `std::perms` as `cxx::perms`.

    ```cpp
    #include "iceoryx_hoofs/cxx/filesystem.hpp"

    // ...
    cxx::perms filePermissions;
    filePermissions = cxx::perms::owner_read | cxx::perms::group_write;
    std::cout << filePermissions << std::endl;
    ```

**API Breaking Changes:**

1. The CMake files in iceoryx expect to have CMake version 3.16 or greater installed, otherwise the build fails.
   (Hint: Ubuntu 18 users can install `cmake-mozilla` from the universe repository provided by Canonical)

1. Dependency for building the `iceoryx_dds` gateway changed from `openjdk` (Java) to `bison`

1. Change include from `iceoryx_hoofs/cxx/helplets.hpp` to `iceoryx_hoofs/cxx/requires.hpp`
   when using `cxx::Expects` or `cxx::Ensures`

    ```cpp
    // before
    #include "iceoryx_hoofs/cxx/helplets.hpp"

    iox::cxx::Expects(someCondition);
    iox::cxx::Ensures(anotherCondition);

    // after
    #include "iceoryx_hoofs/cxx/requires.hpp"

    iox::cxx::Expects(someCondition);
    iox::cxx::Ensures(anotherCondition);
    ```

1. Replace Creation pattern from `MemoryMap` with `MemoryMapBuilder`.

    ```cpp
    // before
    auto memoryMapResult = posix::MemoryMap::create(baseAddress, length, fileDescriptor, accessMode, flags, offset);

    // after
    auto memoryMapResult = posix::MemoryMapBuilder().baseAddressHint(baseAddress)
                                                    .length(length).fileDescriptor(fileDescriptor)
                                                    .accessMode(accessMode).flags(flags)
                                                    .offset(0).create();
    ```

1. Rename utils to hoofs:

    In CMake you need now to find and link the package `iceoryx_hoofs` instead of `iceoryx_utils`

    ```cmake
    # before
    find_package(iceoryx_utils REQUIRED)
    target_link_libraries(${target}
        iceoryx_utils::iceoryx_utils)

    # after
    find_package(iceoryx_hoofs REQUIRED)
    target_link_libraries(${target}
        iceoryx_hoofs::iceoryx_hoofs)
    ```

    The include paths for `iceoryx_utils` are now `iceoryx_hoofs`

    ```cpp
    // before
    #include "iceoryx_utils/cxx/string.hpp"

    // after
    #include "iceoryx_hoofs/cxx/string.hpp"
    ```

1. Refactoring SmartC:

    - Renaming SmartC wrapper to posixCall.
    - Removed `getErrorString()` from posixCall, please use `getHumanReadableErrnum()` instead.
    - Enhanced posixCall to handle a common case were multiple errnos are ignored just to suppress error logging

    ```cpp
    // before
    #include "iceoryx_utils/cxx/smart_c.hpp"

    auto unlinkCallPublisher = iox::cxx::makeSmartC(
        unlink, iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {ENOENT}, sockAddrPublisher.sun_path);

        if (unlinkCallPublisher.hasErrors())
        {
            std::cout << "unlink error" << std::endl;
            exit(1);
        }

    // after
    #include "iceoryx_utils/posix_wrapper/posix_call.hpp"

    iox::posix::posixCall(unlink)(sockAddrPublisher.sun_path)
        .failureReturnValue(ERROR_CODE)
        .ignoreErrnos(ENOENT, EBUSY) // can be a comma-separated list of errnos
        .evaluate()
        .or_else([](auto& r) {
            std::cout << "unlink error " << r.getHumanReadableErrnum() << std::endl;
            exit(1);
        });
    ```

1. Refactoring of `ServiceDescription`

    - A `ServiceDescription` is now only string-based and no more wildcards are allowed.
    - A well-defined `ServiceDescription` consists of three non-empty strings.

    ```cpp
    // before
    ServiceDescription myServiceDescription1(1U, 2U, 3U);
    ServiceDescription myServiceDescription3("First", "Second");
    ServiceDescription myServiceDescription3(iox::capro::AnyServiceString, iox::capro::AnyInstanceString, iox::capro::AnyEventString);

    // after
    ServiceDescription myServiceDescription1("Foo", "Bar", "Baz");
    ServiceDescription myServiceDescription2("First", "Second", "DontCare");
    ServiceDescription myServiceDescription3("Foo", "Bar", "Baz");
    ```

1. Instead of using a constructor a `ServiceDescription` is now deserialized via a
   static method with error handling:

    ```cpp
    // before
    iox::cxx::Serialization serializedObj;
    iox::capro::ServiceDescription service(serializedObj);

    // after
    iox::cxx::Serialization serialisedObj;
    capro::ServiceDescription::deserialize(serialisedObj)
        .and_then([](auto& value){
            // Do something with the deserialized object
        })
        .or_else([](auto& error){
            // Handle the error
        });
    ```

1. The `InvalidIdString` was removed from `ServiceDescription` and the Wildcard string was replaced
   with a `iox::cxx::nullopt`. With this, every string is allowed within the `ServiceDescription`.
   The default `ServiceDescription` consists of empty strings.

1. The service-related methods have been moved from `PoshRuntime` to `ServiceDiscovery`. The `offerService`
   and `stopOfferService` methods have been removed and `findService` has now an additional event parameter.
   Furthermore it requires a function to be provided which is applied to each `ServiceDescription` in
   the search result (and can be used to collect them in a container etc.).

   The `iox::popo::MessagingPattern` parameter allows to search publishers (`PUB_SUB`) or
   servers (`REQ_RES`).

    ```cpp
    // before
    #include "iceoryx_posh/runtime/posh_runtime.hpp"

    poshRuntime.offerService(myServiceDescription);
    poshRuntime.stopOfferService(myServiceDescription);
    poshRuntime.findService({"ServiceA", iox::capro::AnyInstanceString});

    // after
    #include "iceoryx_posh/runtime/service_discovery.hpp"

    void printSearchResult(const iox::capro::ServiceDescription& service)
    {
        std::cout << "- " << service << std::endl;
    }

    serviceDiscovery.findService("ServiceA", Wildcard, Wildcard, printSearchResult, iox::popo::MessagingPattern::PUB_SUB);
    ```

1. The following classes have now an constructor marked as `explicit`:

    ```cpp
    explicit DeadlineTimer(const iox::units::Duration timeToWait);
    explicit GenericRAII(const std::function<void()>& cleanupFunction);
    explicit mutex(const bool f_isRecursive);
    explicit PosixUser(const uid_t f_id);
    explicit PosixUser(const string_t& f_name);
    ```

1. Renaming in `FileReader` class and logging of iceoryx_hoofs

    ```cpp
    // before
    iox::cxx::FileReader reader("filename");
    std::string str;
    if(reader.IsOpen()) {
        reader.ReadLine(str);
    }

    static auto& logger = CreateLogger("", "", iox::log::LogManager::GetLogManager().DefaultLogLevel());

    // after
    iox::cxx::FileReader reader("filename");
    std::string str;
    if(reader.isOpen()) {
        reader.readLine(str);
    }

    static auto& logger = createLogger("", "", iox::log::LogManager::GetLogManager().DefaultLogLevel());
    ```

1. The `iox::cxx::expected` has dropped the requirement for `INVALID_STATE`. With this, the
   `ErrorTypeAdapter` which was necessary for non enum types was also removed. The specialization
   of `ErrorTypeAdapter` for custom types must therefore also be removed in the user code.

1. The queue port policy enums are adjusted to use them with `Client` and `Server`.

1. The `QueueFullPolicy::BLOCK_PUBLISHER` is replaced with the more generic `QueueFullPolicy::BLOCK_PRODUCER`.

    ```cpp
    // old
    iox::popo::SubscriberOptions options;
    options.queueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PUBLISHER;

    // new
    iox::popo::SubscriberOptions options;
    options.queueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    ```

    Similar, for `binding_c` it is `QueueFullPolicy_BLOCK_PRODUCER` instead of `QueueFullPolicy_BLOCK_PUBLISHER`

    ```c
    // old
    iox_sub_options_t options;
    iox_sub_options_init(&options);
    options.queueFullPolicy = QueueFullPolicy_BLOCK_PUBLISHER;

    // new
    iox_sub_options_t options;
    iox_sub_options_init(&options);
    options.queueFullPolicy = QueueFullPolicy_BLOCK_PRODUCER;
    ```

1. The `SubscriberTooSlowPolicy` is replaced with the more generic `ConsumerTooSlowPolicy` and
   `SubscriberTooSlowPolicy::WAIT_FOR_SUBSCRIBER` became `ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER`.

    ```cpp
    // old
    iox::popo::PublisherOptions options;
    options.subscriberTooSlowPolicy = iox::popo::SubscriberTooSlowPolicy::WAIT_FOR_SUBSCRIBER;

    // new
    iox::popo::PublisherOptions options;
    options.subscriberTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
    ```

    And with the `binding_c`

    ```c
    // old
    iox_pub_options_t options;
    iox_pub_options_init(&options);
    options.subscriberTooSlowPolicy = SubscriberTooSlowPolicy_WAIT_FOR_SUBSCRIBER;

    // new
    iox_pub_options_t options;
    iox_pub_options_init(&options);
    options.subscriberTooSlowPolicy = ConsumerTooSlowPolicy_WAIT_FOR_CONSUMER;
    ```

1. The `CaproMessageSubType` enum is renamed to `CaproServiceType` and the values are renamed from `NOSUBTYPE`,
  `SERVICE`, `EVENT` and `FIELD` to `NONE`, `PUBLISHER` and `SERVER`.
  This change only affects `InterfacePorts` which used this enum to communicate whether the `CaproMessage`
  was from a `SERVICE`, `EVENT` or `FIELD`. This was quite ara::com specific and with the introduction of the `ServerPort`
  changes were needed. The distinction between a `FIELD` and an `EVENT` can be made by checking
  `CaproMessage::m_historyCapacity`.

    ```cpp
    // old
    caproMessage.m_subType = CaproMessageSubType::EVENT;

    // new
    caproMessage.m_serviceType = CaproServiceType::PUBLISHER;
    ```
