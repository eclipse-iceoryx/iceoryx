#!/bin/bash

ICEORYX_ROOT=$(git rev-parse --show-toplevel)
VERIFICATION_SUCCESS=1
VERIFIED_ISSUE_CACHE=(0)
FAILED_ISSUE_CACHE=(0)

cd $ICEORYX_ROOT

FILES_WITH_TODOS=$(grep -i -RIne "todo" | grep -v doc/ | grep -v .git/ | grep -v build/ | grep -v .sh)

is_issue_in_verified_cache() {
    IS_IN_VERIFIED_CACHE=0
    local ISSUE_NUMBER=$1
    for N in "${VERIFIED_ISSUE_CACHE[@]}"
    do
        if [[ $N -eq $ISSUE_NUMBER ]]
        then
            IS_IN_VERIFIED_CACHE=1
            return
        fi
    done
}

is_issue_in_failed_cache() {
    IS_IN_FAILED_CACHE=0
    local ISSUE_NUMBER=$1
    for N in "${FAILED_ISSUE_CACHE[@]}"
    do
        if [[ $N -eq $ISSUE_NUMBER ]]
        then
            IS_IN_FAILED_CACHE=1
            return
        fi
    done
}

is_todo_compliant() {
    IS_COMPLIANT=1
    HAS_VALID_ISSUE_NUMBER=1

    local LINE=$1
    local ISSUE_NUMBER=$( echo $LINE | sed -n "s/.*\@todo\ iox-\#\([0-9]*\).*/\1/p" )

    if [[ -z $ISSUE_NUMBER ]]
    then
        IS_COMPLIANT=0
        VERIFICATION_SUCCESS=0
        return
    fi

    is_issue_in_verified_cache $ISSUE_NUMBER
    if [[ $IS_IN_VERIFIED_CACHE -eq 1 ]]
    then
        return
    fi

    is_issue_in_failed_cache $ISSUE_NUMBER
    if [[ $IS_IN_FAILED_CACHE -eq 1 ]]
    then
        HAS_VALID_ISSUE_NUMBER=0
        VERIFICATION_SUCCESS=0
        return
    fi

    if wget --spider https://github.com/eclipse-iceoryx/iceoryx/issues/$ISSUE_NUMBER 2>/dev/null
    then
        VERIFIED_ISSUE_CACHE+=($ISSUE_NUMBER)
    else
        FAILED_ISSUE_CACHE+=($ISSUE_NUMBER)
        HAS_VALID_ISSUE_NUMBER=0
        VERIFICATION_SUCCESS=0
        return
    fi
}


OLDIFS=$IFS
IFS=$'\n'
for ENTRY in ${FILES_WITH_TODOS}
do
    SOURCE=$(echo $ENTRY | sed -n "s/\([^:]*:[0-9]*\):.*/\1/p")
    LINE=$(echo $ENTRY | sed -n "s/[^:]*:[0-9]*:\(.*\)/\1/p")

    is_todo_compliant $LINE

    if [[ $IS_COMPLIANT -eq 0 ]]
    then
        echo "$SOURCE ::: False todo syntax! It must follow \"@todo iox-#?\" where ? marks a valid issue number."
        echo "  $LINE"
        echo
    fi

    if [[ $HAS_VALID_ISSUE_NUMBER -eq 0 ]]
    then
        echo "$SOURCE ::: Todo points to non-existing github issue."
        echo "  $LINE"
        echo
    fi
done
IFS=$OLDIFS
