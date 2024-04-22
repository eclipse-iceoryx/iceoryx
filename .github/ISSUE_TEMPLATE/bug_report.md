---
name: Bug report
about: Create a report to help us improve
title: ''
labels: ''
assignees: ''

---

## Required information

**Operating system:**
E.g. Ubuntu 18.04 LTS

**Compiler version:**
E.g. GCC 7.4.0

**Eclipse iceoryx version:**
E.g. `v1.2.3` or `main` branch

**Observed result or behaviour:**
A clear and precise description of the observed result.

**Expected result or behaviour:**
What do you expect to happen?

**Conditions where it occurred / Performed steps:**
Describe how one can reproduce the bug.

## Additional helpful information

If there is a core dump, please run the following command and add the output to the issue in a separate comment

```console
gdb --batch \
   --ex "shell printf '\n\033[33m#### Local Variables ####\033[m\n'"  --ex "info locals" \
   --ex "shell printf '\n\033[33m#### Threads ####\033[m\n'"          --ex "info threads" \
   --ex "shell printf '\n\033[33m#### Shared Libraries ####\033[m\n'" --ex "info sharedlibrary" \
   --ex "shell printf '\n\033[33m#### Stack Frame ####\033[m\n'"      --ex "info frame" \
   --ex "shell printf '\n\033[33m#### Register ####\033[m\n'"         --ex "info register" \
   --ex "shell printf '\n\033[33m#### Backtrace ####\033[m'"          --ex "thread apply all bt" \
   --core coreDumpFile binaryFile
```
