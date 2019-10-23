# Contributing to Eclipse iceoryx

Thanks for your interest in this project.

## Project description

In domains like automotive, robotics or gaming, a huge amount of data must be
transferred between different parts of the system. If these parts are actually
different processes on a POSIX based operating system like Linux, this huge
amount of data has to be transferred via an inter-process-communication (IPC)
mechanism.

* https://projects.eclipse.org/projects/technology.iceoryx

## Developer resources

Information regarding source code management, builds, coding standards, and
more.

* https://projects.eclipse.org/projects/technology.iceoryx/developer

The project maintains the following source code repositories

* https://github.com/eclipse/iceoryx

## Eclipse Contributor Agreement

Before your contribution can be accepted by the project team contributors must
electronically sign the Eclipse Contributor Agreement (ECA).

* http://www.eclipse.org/legal/ECA.php

Commits that are provided by non-committers must have a Signed-off-by field in
the footer indicating that the author is aware of the terms by which the
contribution has been provided to the project. The non-committer must
additionally have an Eclipse Foundation account and must have a signed Eclipse
Contributor Agreement (ECA) on file.

For more information, please see the Eclipse Committer Handbook:
https://www.eclipse.org/projects/handbook/#resources-commit

## Contact

Contact the project developers via the project's "dev" list.

* iceoryx-dev@eclipse.org

## Feature request and bugs

We love pull requests! The next sections try to cover most of the relevant questions. For larger contributions or
architectural changes we'd kindly ask you to get in touch with one of the maintainers beforehand. If you would like to
report a bug or propose a new feature, please raise an issue before raising a pull request. This makes it easier to
track. Beforehand, please make sure you have:

1. Signed the [ECA](http://www.eclipse.org/legal/ECA.php)
2. All commits have been commited with `git commit -s`

## Coding style

We love the [C++ core guidelines](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines). If in doubt please try
to follow them as well as our unwritten conventions in the exisiting parts of the code base.
Please format your code with the provided [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and
[clang-tidy](https://clang.llvm.org/extra/clang-tidy/) before raising a pull request. Lots of IDEs do read the
clang-format file these days.

We created some convenient rules to highlite some bits that you might not be used to in other FOSS projects. They are
helpful to build embedded systems for safety fields like automotive or avionics. It is possible that not the whole
codebase follows these rules, things are work in progress.

1) **No heap is allowed**, static memory management hugely decreases the complexity of your software (e.g. cxx::vector
    without heap)
2) **No exception are allowed**, all function and methods need to have `noexcept` in their signature
3) **No undefined behavior**, zero-cost abstract is not feasible in high safety environments
4) **Use C++11**, however we try to introduce C++14 as fast as possible
5) **[Rule of Five](https://en.wikipedia.org/wiki/Rule_of_three_(C%2B%2B_programming))**, if there is a non-default
    destructor needed, the rule of five has to be applied
6) **[STL](https://en.wikipedia.org/wiki/Standard_Template_Library)**, we aim to be compatible towards the STL, but
    our code may contain additions which are not compatible with the STL (e.g. `iox::cxx::vector::emplace_back()`
    does return a bool)
7) **Always use `iox::log::Logger`**, instead of `printf()`
8) **Always use `iox::ErrorHandler()`**, instead of the direct STL calls
9) **No include guards** Till [modules](https://isocpp.org/files/papers/n4720.pdf) are arriving with C++20, we'll rely
    on `#pragma once` instead of the usual include guards

### Naming conventions

* File names with `lower_snake_case`: `my_thing.hpp`
* Structs, classes and enum classes in `UpperCamelCase`: `class MyClass{}`
* Methods and variables in `lowerCamelCase`: `uint16_t myVariable`
* Compile time constants, also enum values in `UPPER_SNAKE_CASE`: `static constexpr uint16_t MY_CONSTANT`
* Class members start with `m_`: `m_myMember`
* Namespaces in `lower_snake_case` : `my_namespace`

### Doxygen

Please use [doxygen](http://www.doxygen.nl/) to document your code.

The following doxygen comments are required for public API headers:

    /// @brief short description
    /// @param[in] / [out] / [in,out] name description
    /// @return description

## Folder structure

The folder structure boils down to:

* iceoryx_COMPONENT
  * cmake: All cmakes files go here, needed for `find_pkg()`
  * doc: Manuals and documentation
  * include: public headers with stable API
    * internal: public headers with unstable API, which might change quite frequently
  * source: implementation files
  * test: unit and integrations tests
  * CMakeLists.txt: Build the component separately
* examples_iceoryx: Examples described in the main [Readme.md](./README.md#user-content-examples)

All new code should follow the folder structure.

## Testing

We use [Google test](https://github.com/google/googletest) for our unit and integration tests. We require compatibility
with the version 1.8.1.

### Unit tests (aka module tests)

Units tests are black box tests, that test the public interface of a class. They are required for all new code.

### Integration tests

Integration tests are composition of more than one class and test their interaction. They are optional for new code.

## Legal & Compliance

### Dependencies

* [POSIX](https://en.wikipedia.org/wiki/POSIX)
Iceoryx aims to be fully POSIX-compliant towards the current revision POSIX.1-2017 (IEEE 1003.1-2017). Please write
your code as portable as possible. Currently our focus is [QNX](https://blackberry.qnx.com/en) and Linux.

* [ACL](https://en.wikipedia.org/wiki/Access-control_list)

* [ncurses](https://www.gnu.org/software/ncurses/)

### Safety

We aim for [ASIL-D](https://en.wikipedia.org/wiki/Automotive_Safety_Integrity_Level#ASIL_D) compliance. The
[ISO262](https://en.wikipedia.org/wiki/ISO_26262) is also a good read-up if you want to learn more about automotive
safety. As of now we don't have any continous integration checks implemented but will rely on reviews during the pull
requests.

### Security

A good read-up on security topics with C++ is the
[SEI CERT C++ Coding Standard](https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=88046682). If you are
unsure you can always read-up there and use it as best practise. It is possible that not the whole codebase follows
these rules, things are work in progress. But this is where we want go. As of now we don't have any continous
integration checks implemented but will rely on reviews during the pull requests.

### Header

Each source file needs to have this header:

    // Copyright (c) [year] by [Name of author]. All rights reserved.
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
