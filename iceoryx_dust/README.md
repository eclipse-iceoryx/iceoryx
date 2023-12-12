
# Eclipse iceoryx dust overview

Similar to iceoryx hoofs `iceoryx_dust` (**D**eemed **u**seful **s**oftware **t**hingies) is a library for basic building blocks.
Compared to hoofs classes in `iceoryx_dust`, the difference is that the classes in dust only conform to quality level 2.

There are a wide variety of building blocks
grouped together in categories or namespace, depending on where or how they are used.

| class/file            | description                                                                                                                                                                                                                                                                                                           |
|:---------------------:|:----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|`FileReader`           | Wrapper for opening files and reading them.                                                                                                                                                                                                                                                                           |
|`serialization`        | Implements a simple serialization concept for classes based on the idea presented here [ISOCPP serialization](https://isocpp.org/wiki/faq/serialization#serialize-text-format).                                                                                                                                       |
