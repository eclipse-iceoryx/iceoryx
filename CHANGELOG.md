# Change Log
## [v0.90.0](https://github.com/eclipse-iceoryx/iceoryx/tree/v0.90.0) (2020-12-22)
[Full Changelog](https://github.com/eclipse-iceoryx/iceoryx/compare/v0.17.0...v0.90.0)\
Pre-Release for new and stable APIs and n:m pub/sub communication

This is a pre-release for our first long-term-support release that is coming soon (will be iceoryx_1.0.0).

We had a major refactoring of the iceoryx communication infrastructure which allows to do n:m communication now and provides a flexible history functionality for late joining subscribers.

This new infrastructure is also made to support request/response communication as a next step [(\#27)](https://github.com/eclipse-iceoryx/iceoryx/issues/27). The C++ API has been completely reworked and a new C API has been added.

We will give the API draft some weeks for finalizing the features and to incorporate feedback from the community. There's an issue in which we collect and discuss the final modifications [(\#408)](https://github.com/eclipse-iceoryx/iceoryx/issues/408).

**Features:**
- Complete refactoring of publishers and subscribers from shared memory data structures to user APIs, [\#252](https://github.com/eclipse-iceoryx/iceoryx/issues/252)
- First versions of new APIs for C++ and C [\#252](https://github.com/eclipse-iceoryx/iceoryx/issues/252)
- n:m publish/subscribe communication now possible [\#25](https://github.com/eclipse-iceoryx/iceoryx/issues/25)
- First version of a DDS gateway. Cyclone DDS already integrated, FastDDS on it's way [\#64](https://github.com/eclipse-iceoryx/iceoryx/issues/64), [\#65](https://github.com/eclipse-iceoryx/iceoryx/issues/65)
- New github actions for Mac OS and colcon build [\#175](https://github.com/eclipse-iceoryx/iceoryx/issues/175), [\#276](https://github.com/eclipse-iceoryx/iceoryx/issues/276), [\#328](https://github.com/eclipse-iceoryx/iceoryx/issues/328)
- Adjustable capacity for the lockfree queue [\#216](https://github.com/eclipse-iceoryx/iceoryx/issues/216)
- Check the files have a copyright header [\#346](https://github.com/eclipse-iceoryx/iceoryx/issues/346)

**Refactoring:**
- Refactoring of waitset [\#341](https://github.com/eclipse-iceoryx/iceoryx/issues/341)
- create multi publisher example [\#394](https://github.com/eclipse-iceoryx/iceoryx/issues/394)
- Full IceOryx Public API Cheat Sheet [\#283](https://github.com/eclipse-iceoryx/iceoryx/issues/283)
- Rework build and test steps in iceoryx [\#433](https://github.com/eclipse-iceoryx/iceoryx/issues/433)
- Extend iceperf example to C API [\#453](https://github.com/eclipse-iceoryx/iceoryx/issues/453)
- Remove default parameter from PoshRuntime::getInstance() [\#382](https://github.com/eclipse-iceoryx/iceoryx/issues/382)
- Enable Sanitizer in Debug Build and Unit Tests [\#141](https://github.com/eclipse-iceoryx/iceoryx/issues/141)
- Minor RouDi cleanups [\#91](https://github.com/eclipse-iceoryx/iceoryx/issues/91)
- C++14 [\#220](https://github.com/eclipse-iceoryx/iceoryx/issues/220)
- Replace occurence of std::list by cxx::list [\#221](https://github.com/eclipse-iceoryx/iceoryx/issues/221)

**Fixed bugs:**
- SegFault in iox-roudi on startup [\#447](https://github.com/eclipse-iceoryx/iceoryx/issues/447)
- Mocks can cause segfaults/undefined behavior [\#427](https://github.com/eclipse-iceoryx/iceoryx/issues/427)
- Chunks are lost forever when having an overflow in a variant queue of type FIFO [\#456](https://github.com/eclipse-iceoryx/iceoryx/issues/456)
- RouDi does not free shared memory properly on x86 Linux [\#324](https://github.com/eclipse-iceoryx/iceoryx/issues/324)
- WaitSet behavior wrong in `waitAndReturnFulfilledConditions` [\#388](https://github.com/eclipse-iceoryx/iceoryx/issues/388)
- Incorrect usage of strncpy [\#374](https://github.com/eclipse-iceoryx/iceoryx/issues/374)
- Global Instantiation of Publisher/Subscriber created core dump [\#327](https://github.com/eclipse-iceoryx/iceoryx/issues/327)

## [v0.17.0](https://github.com/eclipse-iceoryx/iceoryx/tree/ICEORYX_0.17.0) (2020-08-27)
[Full Changelog](https://github.com/eclipse-iceoryx/iceoryx/compare/ICEORYX_0.17.0_RC6...v0.17.0)\
MacOS support and preparations for new API

**Packages:**
- iceoryx_posh (Quality level 4)
- iceoryx_utils (Quality level 4)
- iceoryx_examples (Quality level 5)

**Features:**
- MacOS support [\#32](https://github.com/eclipse-iceoryx/iceoryx/issues/32)
- Major RouDi refactorings [\#70](https://github.com/eclipse-iceoryx/iceoryx/issues/70), [\#59](https://github.com/eclipse-iceoryx/iceoryx/issues/59), [\#78](https://github.com/eclipse-iceoryx/iceoryx/issues/78)
- Preparations for new iceoryx API [\#25](https://github.com/eclipse-iceoryx/iceoryx/issues/25)
- iceoryx to cyclonedds gateway [\#64](https://github.com/eclipse-iceoryx/iceoryx/issues/64)
- Introduce cxx::function_ref [\#86](https://github.com/eclipse-iceoryx/iceoryx/issues/86)

**Fixed bugs:**
- POSIX timer improvements [\#120](https://github.com/eclipse-iceoryx/iceoryx/issues/120), [\#167](https://github.com/eclipse-iceoryx/iceoryx/issues/167)
- Memory Synchronisation Issue in FiFo [\#119](https://github.com/eclipse-iceoryx/iceoryx/issues/119)
- Roudi MessageQueue thread startup [\#171](https://github.com/eclipse-iceoryx/iceoryx/issues/171)
- Destructor fix MessageQueue and UnixDomainSocket [\#150](https://github.com/eclipse-iceoryx/iceoryx/issues/150)
- RouDi ressource clean-up [\#113](https://github.com/eclipse-iceoryx/iceoryx/issues/113)

## [v0.17.0_RC6](https://github.com/eclipse-iceoryx/iceoryx/tree/ICEORYX_0.17.0_RC6) (2020-07-29)
[Full Changelog](https://github.com/eclipse-iceoryx/iceoryx/compare/ICEORYX_0.17.0_RC5...ICEORYX_0.17.0_RC6)\
Release Candidate for 0.17.0 release

**Fixed bugs:**
- Sporadic timing test failure [\#120](https://github.com/eclipse-iceoryx/iceoryx/issues/120)
- Improvement: Merge Mempool Introspection data into one sample [\#210](https://github.com/eclipse-iceoryx/iceoryx/issues/210)

## [v0.17.0_RC5](https://github.com/eclipse-iceoryx/iceoryx/tree/ICEORYX_0.17.0_RC5) (2020-07-29)
[Full Changelog](https://github.com/eclipse-iceoryx/iceoryx/compare/ICEORYX_0.17.0_RC4...ICEORYX_0.17.0_RC5)\
Release Candidate for 0.17.0 release

**Fixed bugs:**
- Callable can outrun periodicity of POSIX timer [\#161](https://github.com/eclipse-iceoryx/iceoryx/issues/161)

## [v0.17.0_RC4](https://github.com/eclipse-iceoryx/iceoryx/tree/ICEORYX_0.17.0_RC4) (2020-07-29)
[Full Changelog](https://github.com/eclipse-iceoryx/iceoryx/compare/ICEORYX_0.17.0_RC3...ICEORYX_0.17.0_RC4)\
Release Candidate for 0.17.0 release

**Fixed bugs:**
- The destroy method of MessageQueue and UnixDomainSocket does not fully invalidate the object [\#150](https://github.com/eclipse-iceoryx/iceoryx/issues/150)
- Roudi message queue thread startup [\#171](https://github.com/eclipse-iceoryx/iceoryx/issues/171)

## [v0.17.0_RC3](https://github.com/eclipse-iceoryx/iceoryx/tree/ICEORYX_0.17.0_RC3) (2020-07-27)
[Full Changelog](https://github.com/eclipse-iceoryx/iceoryx/compare/ICEORYX_0.17.0_RC2...ICEORYX_0.17.0_RC3)\
Release Candidate for 0.17.0 release

**Features:**
- cmake options for deployment parameter [\#142](https://github.com/eclipse-iceoryx/iceoryx/issues/142)

## [v0.17.0_RC2](https://github.com/eclipse-iceoryx/iceoryx/tree/ICEORYX_0.17.0_RC2) (2020-07-27)
[Full Changelog](https://github.com/eclipse-iceoryx/iceoryx/compare/ICEORYX_0.17.0_RC1...ICEORYX_0.17.0_RC2)\
Release Candidate for 0.17.0 release

**Refactoring:**
- Refactoring of logging [\#88](https://github.com/eclipse-iceoryx/iceoryx/issues/88)
- Remove const_cast wherever possible [\#76](https://github.com/eclipse-iceoryx/iceoryx/issues/76)
- Remove asynchronous service discovery feature [\#90](https://github.com/eclipse-iceoryx/iceoryx/issues/90)
- qacpp-4.5.0-2427 [\#93](https://github.com/eclipse-iceoryx/iceoryx/issues/93)
- Usage of github actions to build pull requests [\#89](https://github.com/eclipse-iceoryx/iceoryx/issues/89)
- Reduce default memory consumption with config and mempools [\#78](https://github.com/eclipse-iceoryx/iceoryx/issues/78)

**Fixed bugs:**
- Sporadic timing test failure [\#120](https://github.com/eclipse-iceoryx/iceoryx/issues/120)
- Make SOFI real size equal to the specified one [\#105](https://github.com/eclipse-iceoryx/iceoryx/issues/105)
- increase padding in introspection [\#117](https://github.com/eclipse-iceoryx/iceoryx/issues/117)
- Increase the process waiting for RouDi timeout to 60 seconds [\#110](https://github.com/eclipse-iceoryx/iceoryx/issues/110)

## [v0.17.0_RC1](https://github.com/eclipse-iceoryx/iceoryx/tree/ICEORYX_0.17.0_RC1) (2020-03-24)
[Full Changelog](https://github.com/eclipse-iceoryx/iceoryx/compare/v0.16.1...ICEORYX_0.17.0_RC1)\
Release Candidate for 0.17.0 release

**Features:**
- Memory abstraction, modularization of RouDi and fixes [\#59](https://github.com/eclipse-iceoryx/iceoryx/issues/59)
- Memory abstraction and RouDi modularization step 2, fixed string improvements and other fixes [\#70](https://github.com/eclipse-iceoryx/iceoryx/issues/70)

**Fixed bugs:**
- Can't compile due to missing CPPTOML [\#67](https://github.com/eclipse-iceoryx/iceoryx/issues/67)


## [v0.16.1](https://github.com/eclipse-iceoryx/iceoryx/tree/v0.16.1) (2020-03-02)
[Full Changelog](https://github.com/eclipse-iceoryx/iceoryx/compare/v0.16.0...v0.16.1)\
Support for rmw_iceoryx with ROS2 python API

**Packages:**

- POSH (POSIX Shared Memory)
- Utils
- Iceoryx examples

**Features:**

- Capability to cleanup shared memory resources during process lifetime [\#51](https://github.com/eclipse-iceoryx/iceoryx/issues/51)

**Fixed bugs:**
- compile error with clang [\#43](https://github.com/eclipse-iceoryx/iceoryx/issues/43)
- CMqInterface unit tests are failing caused by unneeded move operations [\#56](https://github.com/eclipse-iceoryx/iceoryx/issues/56)

**Known limitations:**
- ```RouDi --version``` shows 0.16.0.1 instead of 0.16.1

## [v0.16.0](https://github.com/eclipse-iceoryx/iceoryx/tree/v0.16.0) (2019-12-16)
Introspection, performance test and flexible mapping of shared memory

**Packages:**

- POSH (POSIX Shared Memory)
- Utils
- Iceoryx examples

**Features:**

- Introspection client for live debugging of iceoryx [\#21](https://github.com/eclipse-iceoryx/iceoryx/issues/21)
- Flexible mapping of shared memory in virtual address spaces [\#19](https://github.com/eclipse-iceoryx/iceoryx/issues/19)
- Performance test to measure inter-process latency [\#17](https://github.com/eclipse-iceoryx/iceoryx/issues/17)
- Docker build [\#15](https://github.com/eclipse-iceoryx/iceoryx/issues/15) (Thanks to @Mr-Slippery ) 

**Fixed bugs:**
- Payload size not updated by allocateChunk [\#10](https://github.com/eclipse-iceoryx/iceoryx/issues/10)
- Failure in runnable creation [\#23](https://github.com/eclipse-iceoryx/iceoryx/issues/23)

Command for generating git log for merge commits between two tags
```bash
git log --merges --first-parent master --pretty=format:"%h %<(10,trunc)%aN %C(white)%<(15)%aD%Creset %C(red bold)%<(15)%D%Creset %s" <TAG_BASE>...<TAG_TARGET> > diff_merge_commit.log
```