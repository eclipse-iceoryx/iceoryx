# [NOT YET IMPLEMENTED] Ice0ryx C Language binding

If you are looking for an example take a look at the
[icedelivery on c example](../iceoryx_examples/icedelivery_on_c).

## C API structure

The idea of the C API is to be as close to the C++ API as possible. The idea is 
that the C API looks and feels the same like the C++ API so that you do not have 
to learn Ice0ryx from scratch if you would like to use the C API.

Therefore, we have the following coding conventions exclusively in the C API.

 - C functions are named like `ClassName_MethodName`
    - The constructor is always named like `ClassName_create` and the
      destructor `ClassName_destroy`
    - The first parameter is always the handle to the corresponding class.
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

class MyTransmitter {
    public:
        MyTransmitter(const std::string &name);
        cxx::optional<void*> receive();
        bool send(void * data);
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

CLASS MyTransmitterHandle* MyTransmitter_new(const char * name);
void MyTransmitter_delete(CLASS MyTransmitterHandle * self);
bool MyTransmitter_receive(void * const data);
bool MyTransmitter_send(void * const data);
```

