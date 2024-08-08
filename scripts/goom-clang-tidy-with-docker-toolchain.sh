#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-get-paths.sh"
source "${THIS_SCRIPT_PATH}/goom-get-vars.sh"

declare CLION_ARG=""
if [[ "${USING_CLION}" == "yes" ]]; then
  CLION_ARG="--clion"
fi

echo "Running clang-tidy from container \"${DOCKER_BUILD_IMAGE}\"..."
echo

docker run --rm                                                        \
           -e TZ=${HOST_TIME_ZONE}                                     \
           -e CCACHE_DIR=${DOCKER_CCACHE_DIR}                          \
           -v ${GOOM_MAIN_ROOT_DIR}:${DOCKER_GOOM_MAIN_ROOT_DIR}       \
           -v /tmp:/tmp                                                \
           -t ${DOCKER_BUILD_IMAGE}                                    \
           bash -c "cd ${DOCKER_GOOM_MAIN_ROOT_DIR} &&                 \
                    bash ${DOCKER_GOOM_SCRIPTS_DIR}/goom-clang-tidy.sh \
                      ${CLION_ARG}                                     \
                      --docker                                         \
                      --docker-os-type ${DOCKER_OS_TYPE}               \
                      --docker-os-tag ${DOCKER_OS_TAG}                 \
                      --compiler ${COMPILER}                           \
                      --build-type ${BUILD_TYPE}                       \
                      --suffix ${BUILD_DIR_SUFFIX}                     \
                      $@"

