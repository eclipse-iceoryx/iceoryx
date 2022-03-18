#!/usr/bin/env bash

# Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

#################
## usage hints ##
#################
# * when an URL contains ( ) please replace them with the code %28 %29 otherwise
#   the parser will deliver a false positive
#
# * when a link should not be tested add the comment <!--NOLINT some comment-->
#   in the same line

ICEORYX_ROOT_PATH=$(git rev-parse --show-toplevel)
EXIT_CODE=0

ENABLE_URL_CHECK=1

# skips github issue urls since they tend to fail quiet often
# without reason
SKIP_GITHUB_URL_CHECK=1

FILE_TO_SCAN=$1

setupTerminalColors()
{
    if [[ -t 1 ]] ## output to console, we want colors!
    then
        COLOR_BLACK="\e[30m"
        COLOR_RED="\e[31m"
        COLOR_GREEN="\e[32m"
        COLOR_YELLOW="\e[33m"
        COLOR_BLUE="\e[34m"
        COLOR_MAGENTA="\e[35m"
        COLOR_CYAN="\e[36m"
        COLOR_LIGHT_GRAY="\e[37m"
        COLOR_GRAY="\e[90m"
        COLOR_LIGHT_RED="\e[91m"
        COLOR_LIGHT_GREEN="\e[92m"
        COLOR_LIGHT_YELLOW="\e[93m"
        COLOR_LIGHT_BLUE="\e[94m"
        COLOR_LIGHT_MAGENTA="\e[95m"
        COLOR_LIGHT_MAGENTA="\e[96m"
        COLOR_WHITE="\e[97m"
        COLOR_RESET="\e[0m"
    else ## output into file, no colors and escape codes!
        COLOR_BLACK=""
        COLOR_RED=""
        COLOR_GREEN=""
        COLOR_YELLOW=""
        COLOR_BLUE=""
        COLOR_MAGENTA=""
        COLOR_CYAN=""
        COLOR_LIGHT_GRAY=""
        COLOR_GRAY=""
        COLOR_LIGHT_RED=""
        COLOR_LIGHT_GREEN=""
        COLOR_LIGHT_YELLOW=""
        COLOR_LIGHT_BLUE=""
        COLOR_LIGHT_MAGENTA=""
        COLOR_LIGHT_MAGENTA=""
        COLOR_WHITE=""
        COLOR_RESET=""
    fi
}

doesWebURLExist()
{
    if curl --max-time 10 --insecure --head --silent --fail $1 2> /dev/null 1>/dev/null ;
    then
        echo 1
    else
        echo 0
    fi
}

isWebLink()
{
    local RESULT
    RESULT=${RESULT}$(echo $1 | sed -n "s/^https:\/\/.*//p" | wc -l)
    RESULT=${RESULT}$(echo $1 | sed -n "s/^http:\/\/.*//p" | wc -l)
    echo $RESULT | grep 1 | wc -l
}

isMailLink()
{
    echo $1 | grep -E "^mailto:" | wc -l
}

isLinkToSection()
{
    echo $1 | grep -E "^#" | wc -l
}

isAbsolutePath()
{
    echo $1 | grep -E "^/" | wc -l
}

printLinkFailureSource()
{
    echo -e "  name: ${COLOR_LIGHT_YELLOW} $LINK_NAME ${COLOR_RESET}"
    echo -e "  line: ${COLOR_LIGHT_YELLOW} $LINE_NR ${COLOR_RESET}"
    echo -e "  link: ${COLOR_LIGHT_RED} $LINK${COLOR_RESET}"
    echo

    EXIT_CODE=1
}

verifyLinkToSection()
{
    local LOCAL_LINK_VALUE=$1
    local LOCAL_FILE=$2

    LINK=$(echo $LOCAL_LINK_VALUE | sed -n "s/^#\(.*\)/#\1/p" | sed "s/\./\\\./g" | tr - '.')
    local LOCAL_LINK=$(echo $LINK | cut -f 2 -d '#')
    if ! [[ $(cat $LOCAL_FILE | grep -iE "# $LOCAL_LINK\$" | wc -l ) == 1 ]]
    then
        printLinkFailureSource
    fi
}

verifyLinkToUrl()
{
    if [[ $ENABLE_URL_CHECK == "1" ]]
    then
        LINK=$1

        if [[ $SKIP_GITHUB_URL_CHECK == "1" ]]
        then
            if [[ $(echo $LINK | grep "https://github.com/eclipse-iceoryx/iceoryx/issue" | wc -l) == "1" ]]
            then
                return
            fi
        fi

        # verify that no link to a specific github tree is used in the documentation
        if [[ $(echo $LINK | grep "https://github.com/eclipse-iceoryx/iceoryx/tree/" | wc -l ) == "1" ]]
        then
            local PATH_AFTER_TREE=$(echo $LINK | sed -n "s/https:\/\/github.com\/eclipse-iceoryx\/iceoryx\/tree\/\(.*\)/\1/p")
            if [[ $(echo $PATH_AFTER_TREE | grep "/" | wc -l ) == "1" ]]
            then
                echo -e "${COLOR_LIGHT_RED}Please do not use a github url when also a relative path works!${COLOR_RESET}"
                echo -e "${COLOR_LIGHT_RED}Relative paths ensure that the tag/branch of link source and destination is equal.${COLOR_RESET}"
                printLinkFailureSource
                return
            fi
        fi

        # verify that no link to a specific github branch is used in the documentation
        if [[ $(echo $LINK | grep "https://github.com/eclipse-iceoryx/iceoryx/blob" | wc -l) == "1" ]]
        then
            echo -e "${COLOR_LIGHT_RED}Please do not use a github url when also a relative path works!${COLOR_RESET}"
            echo -e "${COLOR_LIGHT_RED}Relative paths ensure that the tag/branch of link source and destination is equal.${COLOR_RESET}"
            printLinkFailureSource
            return
        fi

        if [[ $(echo $LINK | grep "https://iceoryx.io" | wc -l ) == "1" ]]
        then
            echo -e "${COLOR_LIGHT_RED}Please try to avoid to link directly to the documentation on https://iceoryx.io!${COLOR_RESET}"
            echo -e "${COLOR_LIGHT_RED}This causes switching between multiple sources of the same material.${COLOR_RESET}"
            printLinkFailureSource
            return
        fi

        if ! [[ $(doesWebURLExist $LINK) == "1" ]]
        then
            printLinkFailureSource
        fi
    fi
}

verifyLinkToFile()
{
    LINK_VALUE=$1

    if [[ $(isAbsolutePath $LINK_VALUE) == "1" ]]
    then
        LINK=${ICEORYX_ROOT_PATH}/${LINK_VALUE}
    else
        LINK=${FILE_DIRECTORY}/${LINK_VALUE}
    fi

    if [[ $(echo $LINK | grep '#' | wc -l) == "1" ]]
    then
        SECTION_IN_FILE=$(echo $LINK | cut -f 2 -d '#')
        LINK=$(echo $LINK | cut -f 1 -d '#')
    fi

    if ! [ -f $LINK ] && ! [ -d $LINK ]
    then
        printLinkFailureSource

        POSSIBLE_ALTERNATIVE=$(find $ICEORYX_ROOT_PATH -type f -iname $(basename $LINK))
        echo -e "Is this the file you are looking for: ${COLOR_LIGHT_BLUE}$POSSIBLE_ALTERNATIVE${COLOR_RESET}"
        echo
        SECTION_IN_FILE=""
        return
    fi

    if ! [[ $SECTION_IN_FILE == "" ]]
    then
        verifyLinkToSection "#$SECTION_IN_FILE" $LINK
    fi

    SECTION_IN_FILE=""
}

checkLinksInFile()
{
    FILE=$1

    FILE_DIRECTORY=$(dirname $FILE)
    IS_IN_CODE_ENV="0"

    readarray FILE_CONTENT < $FILE
    LINE_NR=0
    for LINE in "${FILE_CONTENT[@]}"
    do
        let LINE_NR=$LINE_NR+1

        # it is possible to have code environments like
        # ````
        # ```
        # hello world
        # ```
        # ````
        # this is at the moment not supported, only ``` so we print a warning when we
        # encounter such a line
        if [[ $(echo $LINE | grep -E "^[ ]*\`\`\`\`" | wc -l) == "1" ]]
        then
            echo -e ${COLOR_LIGHT_RED}File: $FILE, markdown code environment with more than 3 \` are not supported. You may encounter false positives.${COLOR_RESET}
            continue
        fi

        # detect code environments, see ``` and skip them
        if [[ $(echo $LINE | grep -E "^[ ]*\`\`\`" | wc -l) == "1" ]]
        then
            if [[ $IS_IN_CODE_ENV == "1" ]]
            then
                IS_IN_CODE_ENV="0"
            else
                IS_IN_CODE_ENV="1"
            fi
        fi

        if [[ $IS_IN_CODE_ENV == "1" ]]
        then
            continue
        fi

        # detect NOLINT and skip those lines
        if [[ $(echo $LINE | grep -E "<!--NOLINT.*-->" | wc -l) == "1" ]]
        then
            continue
        fi

        ## sed -e 's/[^[]`[^`]*`//g' 
        ## remove inline code env like `auto bla = [blubb](auto i) ..` which could be mistaken as
        ## a markdown link like [linkName](linkValue)
        ##
        ## sed -n "s/.*\[\(.*\)](\([^)]*\)).*/\1[\2/p"
        ## extract markdown links
        MARKDOWN_LINK=$(echo $LINE | sed -e 's/[^[]`[^`]*`//g' | sed -n "s/.*\[\(.*\)](\([^)]*\)).*/\1[\2/p" | tr ' ' _)
        if [[ $MARKDOWN_LINK == "" ]]
        then
            continue
        fi

        LINK_NAME=$(echo $MARKDOWN_LINK | cut -f 1 -d '[')
        LINK_VALUE=$(echo $MARKDOWN_LINK | cut -f 2 -d '[')

        if [[ $(isMailLink $LINK_VALUE) == "1" ]]
        then
            continue
        elif [[ $(isWebLink $LINK_VALUE) == "1" ]]
        then
            verifyLinkToUrl $LINK_VALUE
        elif [[ $(isLinkToSection $LINK_VALUE) == "1" ]]
        then
            verifyLinkToSection $LINK_VALUE $FILE
        else
            verifyLinkToFile $LINK_VALUE
        fi
    done
}

performLinkCheck()
{
    NUMBER_OF_FILES=$(find $ICEORYX_ROOT_PATH -type f -iname "*.md" | grep -v ${ICEORYX_ROOT_PATH}/build | grep -v ${ICEORYX_ROOT_PATH}/.github | grep -v ${ICEORYX_ROOT_PATH}/.git | wc -l)

    CURRENT_FILE=0
    for FILE in $(find $ICEORYX_ROOT_PATH -type f -iname "*.md" | grep -v ${ICEORYX_ROOT_PATH}/build | grep -v ${ICEORYX_ROOT_PATH}/.github | grep -v ${ICEORYX_ROOT_PATH}/.git)
    do
        let CURRENT_FILE=$CURRENT_FILE+1
        echo -e "[$CURRENT_FILE/$NUMBER_OF_FILES] ${COLOR_LIGHT_GREEN}$FILE${COLOR_RESET}"

        checkLinksInFile $FILE
    done
}

setupTerminalColors

if ! [ -z $1 ]
then
    checkLinksInFile $FILE_TO_SCAN
else
    performLinkCheck
fi

exit $EXIT_CODE
