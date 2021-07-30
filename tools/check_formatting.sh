#!/bin/bash
set -e

fail() {
    printf "\033[1;31merror: %s: %s\033[0m\n" ${FUNCNAME[1]} "${1:-"Unknown error"}"
    exit 1
}

# check clang-format
hash clang-format || fail "clang-format not found"
CLANG_MAJOR_VERSION=$(clang-format --version | awk '{print $(NF)}' | sed 's/\..*$//')
[[ $CLANG_MAJOR_VERSION -ge "10" ]] || fail "clang-format of version 10 or higher is not installed."

cd $(git rev-parse --show-toplevel)

# format files
git ls-files | grep -E "\.(c|cpp|inl|h|hpp)$" | xargs clang-format -i -style=file

# check incorrectly formatted fails and fail
git diff-index --name-only --exit-code HEAD --
