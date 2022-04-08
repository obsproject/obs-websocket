#!/usr/bin/env bash

set -o errexit
set -o pipefail

if [ ${#} -eq 1 -a "${1}" = "VERBOSE" ]; then
    VERBOSITY="-l debug"
else
    VERBOSITY=""
fi

if [ "${CI}" ]; then
    MODE="--check"
else
    MODE="-i"
fi

# Runs the formatter in parallel on the code base.
# Return codes:
#  - 1 there are files to be formatted
#  - 0 everything looks fine

# Get CPU count
OS=$(uname)
NPROC=1
if [[ ${OS} = "Linux" ]] ; then
    NPROC=$(nproc)
elif [[ ${OS} = "Darwin" ]] ; then
    NPROC=$(sysctl -n hw.physicalcpu)
fi

# Discover clang-format
if ! type cmake-format 2> /dev/null ; then
    echo "Required cmake-format not found"
    exit 1
fi

find . -type d \( \
    -path ./\*build\* -o \
    -path ./deps/websocketpp -o \
    -path ./deps/json -o \
    -path ./deps/json \
\) -prune -false -type f -o \
    -name 'CMakeLists.txt' -or \
    -name '*.cmake' \
 | xargs -L10 -P ${NPROC} cmake-format ${MODE} ${VERBOSITY}
