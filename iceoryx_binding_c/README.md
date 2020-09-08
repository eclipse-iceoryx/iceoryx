# [NOT YET IMPLEMENTED] Ice0ryx C Language binding

If you are looking for an example take a look at the
[icedelivery on c example](../iceoryx_examples/icedelivery_on_c).

## C API structure

The idea of the C API is to be as close to the C++ API as possible. The idea is 
that the C API looks and feels the same like the C++ API so that you do not have 
to learn Ice0ryx from scratch if you would like to use the C API.

Therefore, we have the following coding conventions exclusively in the C API.

 - C functions are using an abbreviation of the `ClassName` for instance `cn` and
      are named like `iox_cn_method_name` where the camelCase is converted into 
      snake_case.
    - The constructor has always a suffix `_create` and is called for instance 
        `cn_create`. Analog to the constructor the destructor has the suffix `_destroy` 
        and is named like `cn_destroy`.
    - The first parameter is always the handle to the corresponding object.
    - If possible, the arguments should stay the same in the C API.

 - Every C++ class has the prefix keyword `CLASS` in the C header.
   The reason is that C requires the `struct` keyword and in C++ we
   only require the type. Hence, we defined a macro which adds 
   `struct` before every C++ class when building a C project.
 - Enum values are named like `EnumName_EnumValue`
 - Enum names follow the rule `namespace_EnumName`

Here is an example:
```cpp
namespace iox {
enum class Color {
    RED,
    GREEN,
    BLUE
};

class Subscriber {
    public:
        Subscriber(const std::string &name);
        cxx::optional<void*> receive();
        bool send(void * data);
        void allHailHypnotoad();
};
}
```

The corresponding C binding would then look like:
```c
enum iox_Color {
    Color_RED,
    Color_GREEN,
    Color_BLUE,
};

CLASS MyTransmitterHandle* iox_sub_create(const char * name);
void iox_sub_destroy(CLASS MyTransmitterHandle * self);
bool iox_sub_receive(void * const data);
bool iox_sub_send(void * const data);
void iox_sub_all_hail_hypnotoad();
```

