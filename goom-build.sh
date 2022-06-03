#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-set-vars.sh"


if [[ ! -d "${BUILD_DIR}" ]]; then
  echo "ERROR: Could not find build directory \"${BUILD_DIR}\"."
  exit 1
fi

echo

export CC=${C_COMPILER}
export CXX=${CPP_COMPILER}
echo "Using compilers: C: ${C_COMPILER}, C++: ${CPP_COMPILER}."

echo "Using BUILD_DIR: \"${BUILD_DIR}\"."
echo

cmake --build "${BUILD_DIR}" -j 6

echo

cmake --install "${BUILD_DIR}"

echo
echo "Finished cmake build and install in build dir \"${BUILD_DIRNAME}\"."
echo
