# [ASPICE](http://www.automotivespice.com/) example [WIP]

## Purpose

This document defines the documentation guidelines for SWE 3 (Detailed Design) and SWE 4 (Unit Verification).


## ToDo
 - which doxygen tags needs to be set
 - clarify alias for concurrent and error
 - Doxygen plantuml example
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
    void BadCommentedFunction();

    /// good comment: bla bla bla
    void GoodCommentedFunction();
 ```

 * Try to give a usage example via @code / @endcode of more complex to use functions

