# [ASPICE](http://www.automotivespice.com/) example [WIP]

## Purpose

This document defines the documentation guidelines for SWE 3 (Detailed Design) and SWE 4 (Unit Verification).


## ToDo
 - provide which doxygen tags needs to be set
 - clarify alias for concurrent and error
 - write documentation guideline

##  Tags
### Allowed Tags
 - someTag

### Custom Tags

### Forbidden Tags
- unit , use a type safe implementation if possible!
- min , use a type safe implementation if possible!
- max , use a type safe implementation if possible!

#### Howto define a custom tag
Edit the file ```Doxygen``` and add the following alias.
```
ALIASES +="customTag=\par Custom Tag Description:\n"
```
#### Required custom tags
```
ALIASES += "requirement=\xrefitem requirement \"Requirement\" \"Requirements\" "
```

## Howto document your code
 * The implementation documentation should never describe what happens, that does already the code for you. It should describe why it is implemented in the way it is.

 * Never add fancy lines in your comments like this: (exception is the fileheader which is Bosch-wide standard)
 ```cpp
    //=================================
    // My bad comment
    //=================================
    // bla bla bla
    //---------------------------------
    void badCommentedFunction();

    /// good comment: bla bla bla
    void goodCommentedFunction();
 ```
 * Try to give a usage example via @code / @endcode of more complex to use functions

### file header
Please see [Header](https://github.com/eclipse/iceoryx/blob/master/CONTRIBUTING.md#header).

### Include guards
Every header and inl file needs to have an include guard. Pragma once is not allowed.
The include guard is placed directly under the file header and needs to have the following scheme:

   * IOX_[COMPONENT_W/O_ICEORYX][SUB_COMPONENT][FILE_NAME]
   * IOX_[COMPONENT_W/O_ICEORYX][SUB_COMPONENT][SUB_COMPONENT]_[FILE_NAME]

See the header files under iceoryx/doc/aspice_swe3_4/example.

### PlantUML

if you want to make complex aspects of your implementation visible, you can write plantuml directly into the 
doxygen description of your class. An example can be found at example_base_class.hpp

For good examples on PlantUML checkout https://plantuml.com.