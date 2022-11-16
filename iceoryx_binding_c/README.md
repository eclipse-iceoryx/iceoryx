# Iceoryx C Language binding

If you are looking for an example take a look at the
[icedelivery on c example](../iceoryx_examples/icedelivery_in_c).

## C API structure

The C API is supposed to be as close to the C++ API as possible. This means, developers who are
already familiar with the C++ API do not have to learn iceoryx from scratch in order to use the C API.
There are of course differences due to C being a different language than C++.

Therefore, we have the following coding conventions exclusively in the C API.

- C functions are using an abbreviation of the class name. For instance `Subscriber` would use `sub` and
  the methods are named like `iox_sub_method_name` where the `camelCase` is converted into `snake_case`.
- A typedef for the handle is created with the abbreviation as name, a `iox` prefix and a `_t` suffix,
  like `iox_sub_t`
- If the constructor allocates an element it has the suffix `_create` and is called for instance
  `iox_node_create`. Analog to the constructor the destructor has the suffix `_destroy` and is named
  like `iox_node_destroy`.
- Pre iceoryx v2, constructor requiring preallocated memory had the suffix `_init` and required a pointer
  to a storage which was usually placed on the stack. Due to non-trivial issues with different sizes for
  different platforms and architectures the storage is ignored and the object is allocated on the heap.
  This might be reverted in a future release depending on a proper solution for the problem.
  The API remains the same, so a `_storage_t` object like `iox_sub_storage_t` must be created and
  a pointer to this object must be passed to `_init`, e.g. `iox_sub_init`. This function returns a handle
  which does not point to the storage. Using the address of the storage in `_deinit` is undefined and
  most likely will result in a segmentation fault.
  The usage is according the following example

  ```c
  iox_sub_storage_t subStorage;
  iox_sub_t subscriber = iox_sub_init(&subStorage, "foo", "bar", "baz", nullptr);
  // do something with subscriber
  iox_sub_deinit(subscriber);
  ```

- The first parameter is always the handle to the corresponding object.
- If possible, the arguments should stay the same in the C API.

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

class Channel {
    public:
        Channel(const std::string &name);
        optional<void*> receive();
        bool send(void * data);
        void allHailHypnotoad();
};

class MyOtherClass {
    public:
        MyOtherClass(const int a);
        void youSpinMeRoundLikeARecord();
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


typedef struct Channel * sub_t;

sub_t iox_chan_create(const char * name);
void iox_chan_destroy(sub_t const self);
bool iox_chan_receive(sub_t const self, void * const data);
bool iox_chan_send(sub_t const self, void * const data);
void iox_chan_all_hail_hypnotoad(sub_t const self);


struct iox_other_class_storage_t_ {
    uint64_t do_not_touch_me[8]; // lets assume 8 * 8 is the size of MyOtherClass
};
typedef iox_other_class_storage_t_ iox_other_class_storage_t;

typedef struct MyOtherClass * iox_other_class_t;
iox_other_class_t iox_other_class_init(iox_other_class_storage_t * self,const int a);
void iox_other_class_deinit(iox_other_class_t * self);
void iox_other_class_you_spin_me_round_like_a_record(iox_other_class_t * self);
```
