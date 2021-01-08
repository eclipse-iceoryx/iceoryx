## Use Sanitizer Scan

Due to the fact that iceoryx works a lot with system memory it should be ensured that errors like memory leaks are not introduced.
To prevent these, we use the clang toolchain which offers several tools for scanning the codebase. One of them is the [Address-Sanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) which checks for example on dangling pointers.

In iceoryx below sanitizers are enabled at the moment.
- [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)
AddressSanitizer is a fast memory error detector. 
**NOTE :** AddressSanitizer exits on the first detected error, which means there could be more errors in the codebase when this error is reported.
- [LeakSanitizer](https://clang.llvm.org/docs/LeakSanitizer.html)
LeakSanitizer is a run-time memory leak detector. In iceoryx , it runs as part of the AdderssSanitizer.
- [UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
UndefinedBehaviorSanitizer (UBSan) is a fast undefined behavior detector. Iceoryx uses default behaviour ie `print a verbose error report and continue execution`

In iceoryx are scripts available to do the scan on your own. Additionally the Scans are running on the CI in every Pull-Request.
As Requirement you should install the clang compiler:
```
sudo apt install clang
```

Then you need to compile the iceoryx with the sanitizer flags:
```
./tools/iceoryx_build_test.sh build-strict build-all sanitize clang clean
```
After that we can run the tests with enabled sanitizer options:
```
cd build
../tools/run_all_tests.sh
```
When the tests are running without errors then it is fine, else an error report is shown with a stacktrace to find the place where the leak occurs. If the leak comes from an external dependency or shall be handled later then it is possible to set a function on a suppression list.
This should be only rarely used and only in coordination with an iceoryx maintainer.
