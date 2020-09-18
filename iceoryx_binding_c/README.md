# [NOT YET IMPLEMENTED] Ice0ryx C Language binding

If you are looking for an example take a look at the
[icedelivery on c example](../iceoryx_examples/icedelivery_on_c).

## C API structure

The idea of the C API is to be as close to the C++ API as possible. The idea is 
that the C API looks and feels the same like the C++ API so that you do not have 
to learn Ice0ryx from scratch if you would like to use the C API.

Therefore, we have the following coding conventions exclusively in the C API.

 - C functions are using an abbreviation of the class name. For instance `Subscriber` 
      would use `sub` and the methods 
      are named like `iox_sub_method_name` where the camelCase is converted into 
      snake_case.
 - A typedef for the handle is created with the abbreviation as name, a `iox` 
     prefix and a `_t` suffix, like `iox_sub_t`
 - If the constructor allocates an element it has the suffix `_create` and is 
     called for instance `iox_sub_create`. Analog to the constructor the destructor 
     has the suffix `_destroy` and is named like `iox_sub_destroy`.
 - If the constructor requires preallocated memory it has the suffix `_init`
     and is called for instance `iox_sub_init`. The corresponding destructor would 
     then have the suffix `_deinit` and is named like `iox_sub_deinit`.
     - We provide structs to preallocate memory on the stack easily. They are 
         having the suffix `_storage_t` and are named like `iox_sub_storage_t`. This
         allows you to use them like.
         ```c
         iox_sub_storage_t subStorage;
         iox_sub_t = iox_sub_init(&subStorage);
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

class Subscriber {
    public:
        Subscriber(const std::string &name);
        cxx::optional<void*> receive();
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


typedef struct Subscriber * sub_t;

sub_t iox_sub_create(const char * name);
void iox_sub_destroy(sub_t const self);
bool iox_sub_receive(sub_t const self, void * const data);
bool iox_sub_send(sub_t const self, void * const data);
void iox_sub_all_hail_hypnotoad(sub_t const self);


struct iox_other_class_storage_t_ {
    uint64_t do_not_touch_me[8]; // lets assume 8 * 8 is the size of MyOtherClass
};
typedef iox_other_class_storage_t_ iox_other_class_storage_t;

typedef struct MyOtherClass * iox_other_class_t;
iox_other_class_t iox_other_class_init(iox_other_class_storage_t * self,const int a);
void iox_other_class_deinit(iox_other_class_t * self);
void iox_other_class_you_spin_me_round_like_a_record(iox_other_class_t * self);

```

