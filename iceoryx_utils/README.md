# iceoryx utils

The iceoryx utils our basic building blocks - the foundation of 
iceoryx. There are a wide variety of them grouped together in categories
or namespace, depending on where or how they are used.

## structure

 - internal and why, unstable api, maybe soon deprecated or experimental 
   and we would like to use it

## categories

| Namespace       | Short Description |
|:---------------:|:------------------|
| [cxx](..) | Since we are not allowed to use C++17 as well as the heap or exceptions we implemented constructs like ```optional```, ```expected``` or ```variant``` to be as modern as possible. Furthermore, you can find here constructs which are mentioned in the [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines) as well as constructs like ```smart_c``` which enables us to work with good old C function more easily since it does all the error handling for us like taking care of errno or catching EINTR. |
| [concurrent](..) | You should never use concurrent constructs like ```mutex```, ```semaphores```, ```atomic``` etc directly in our codebase. At the moment we still have some exceptions to this guideline but the idea is that all classes which are using them are stored under concurrent and have to undergo more tests then the usual non concurrent class. For instance we try to provide stress tests for them. In here you can find ```fifo```, ```smart_lock```, ```sofi```, ```trigger_queue``` and much more. |
| [design_pattern](..) | Certain code patterns which are repeating themself all over the code are abstracted and stored in here. At the moment we only have the creation pattern but maybe there will be more. |
| [log](..) | The logger used by iceoryx. |
| [posix_wrapper](..) | Posix constructs like shared memory, threads or semaphores are not used directly in our code base. We abstracted them so that they are following the [RAII (Resource Acquisition Is Initialization)](https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization) idiom and other good practices from the C++ community. |
| [units](..) | |
| other | There are even more namespaces inside the iceoryx utils but they will either become obsolete in the future, will be integrated into existing names or are established as another namespace and documented here. We are unsure where they will end up in the future. |

### cxx
STL constructs which are not in the C++11 standard are contained here as
well as constructs which are helping us in our daily life. We differ in
here from our coding guidelines and follow the C++ coding guidelines 
the help the user to have a painless transition from the official STL
type to ours. The API should also be identical to the corresponding
STL type but we have to make exceptions here. For instance, we do not
throw exceptions or are using the heap. In these cases we adjusted the
API to our use case. 

Most of the headers are providing some minimalistic example on how the 
class should be used.

| class                   | internal | description |
|:-----------------------:|:--------:|:------------|
|```algorithm```          |   | Implements ```min``` and ```max``` for an arbitrary number of values of the same type. For instance ```min(1,2,3,4,5);``` |
|```convert```            |   | Converting a number into a string is easy, converting it back can be hard. You can use functions like ```strtoll``` but you still have to handle errors like under- and overflow, or converting invalid strings into number. Here we abstract all the error handling so that you can convert strings into numbers safely. |
|```expected```           |   | Our base class used in error handling. Every function which can fail should return an expected. With this the user knows that this function can fail and that they have to do some kind of error handling. We got inspired by the [C++ expected proposal]( http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0323r7.html) and by the [rust error handling concept](https://doc.rust-lang.org/std/result/enum.Result.html). |
|```GenericRAII```        |   | This is an abstraction of the C++ RAII idiom. Sometimes you have constructs where you would like to perform a certain task on creation and then again when they are getting out of scope, this is where ```GenericRAII``` comes in. It is like a ```std::lock_guard``` or a ```std::shared_ptr``` but more generic. |
|```helplets```           |   | Implementations of [C++ Core Guideline](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) concepts like ```not_null```, ```Ensures```, ```Expects``` are contained here. Additionally, we are providing some types to verify preconditions at compile time. Think of an int which has to be always greater 5 here we provide types like ```greater_or_equal<int, 6>```.|
|```pair```               | x | Simplistic reimplementation of an ```std::pair```. It maybe becomes obsolete. |
|```poor_mans_heap```     |   | |
|```ReferenceCounter```   | x | Basic building block for classes which are needing some kind of reference counting like a ```std::shared_ptr``` |
|```serialization```      |   | |
|```set```                | x | |
|```smart_c```            |   | |
|```string```             |   | |
|```types```              |   | |
|```variant```            |   | |
|```vector```             |   | |

### concurrent

| class                   | internal | description |
|:-----------------------:|:--------:|:------------|
|```ActiveObject```       | x | |
|```FiFo```               | x | |
|```LockedLoFFLi```       | x | |
|```smart_lock```         | x | |
|```SoFi```               | x | |
|```TACO```               | x | |
|```TriggerQueue```       | x | |

### design pattern

| class                   | internal | description |
|:-----------------------:|:--------:|:------------|
|```Creation```           |  | |

### log
logging_free_function_building_block.hpp
| class                   | internal | description |
|:-----------------------:|:--------:|:------------|
|```logger```             |   | |

### posix wrapper

| class                   | internal | description |
|:-----------------------:|:--------:|:------------|
|```AccessController```   | x | |
|```ArgvInspector```      | x | |
|```IpcChannel```         | x | |
|```MessageQueue```       | x | |
|```mutex```              | x | |
|```posix_access_rights```|   | |
|```Semaphore```          |   | |
|```shared_memory_object/Allocator``` | x | |
|```shared_memory_object/MemoryMap``` | x | |
|```shared_memory_object/SharedMemory``` | x | |
|```SharedMemoryObject``` | x | |
|```system_configuration``` | x | |
|```Timer```              |   | |
|```timespec```           | x | |
|```UnixDomainSocket```   | x | |

### units

| class                   | internal | description |
|:-----------------------:|:--------:|:------------|
|```Duration```           | x | |