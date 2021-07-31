#!/bin/bash
set -e

fail() {
    printf "\033[1;31merror: %s: %s\033[0m\n" ${FUNCNAME[1]} "${1:-"Unknown error"}"
    exit 1
}

hash run-clang-tidy-10.py || fail "run-clang-tidy-10.py not found"
cd $(git rev-parse --show-toplevel)
[ -f build/compile_commands.json ] || fail "build/compile_commands.json not found"
# TODO: only process files change in PR
# https://dev.to/scienta/get-changed-files-in-github-actions-1p36
FILES=$(git ls-files | grep -E "\.(c|cpp)$" | grep -E -v '/(test|testing)/')
run-clang-tidy-10.py -p build $FILES
