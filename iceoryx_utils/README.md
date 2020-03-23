# iceoryx utils

The iceoryx utils our basic building blocks - the foundation of 
iceoryx. There are a wide variety of them grouped together in categories
or namespace, depending on where or how they are used.

## structure

 - internal and why

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

| class                   | internal | description |
|:-----------------------:|:--------:|:------------|
|```algorithm```          |   | Implements ```min``` and ```max``` for an arbitrary number of values of the same type. For instance ```min(1,2,3,4,5);``` |
|```convert```            |   | Converting a number into a string is easy, converting it back can be hard. You can use functions like ```strtoll``` but you still have to handle errors like under- and overflow, or converting invalid strings into number. Here we abstract all the error handling so that you can convert strings into numbers safely. |
|```expected```           |   | |
|```GenericRAII```        |   | |
|```helplets```           |   | |
|```pair```               | x | |
|```poor_mans_heap```     |   | |
|```ReferenceCounter```   | x | |
|```serialization```      |   | |
|```set```                | x | |
|```smart_c```            |   | |
|```string```             |   | |
|```types```              |   | |
|```variant```            |   | |
|```vector```             |   | |

### concurrent

### design pattern

### log

### posix wrapper

### units