# complexdata

## Introduction

- to implement zero-copy data transfer we use a shared memory approach
- this requires that every data structure needs to be stored in the shared memory and that
  they do not use the heap (link to restrictions chapter in overview.md)
- most of the STL types cannot be used, but we reimplemented some (link to utils readme/cxx)
- this example shows a) how to send/receive a iox::cxx::vector b) how to send/receive a
  complex data structure containing some of our STL container surrogates

## Expected Output

<!-- @todo Add expected output with asciinema recording before v1.0-->
<!-- @todo multiple examples described in here, expected output should be in front of every example -->

## Code Walkthrough

### Publisher application sending a iox::cxx::vector

### Subscriber application receiving a iox::cxx::vector

### Publisher application sending a complex data structure

### Subscriber application receiving a complex data structure
