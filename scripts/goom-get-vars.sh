source "${GOOM_MAIN_ROOT_DIR}/docker-toolchains/build-get-vars.sh"


declare GOOM_VAR_CMD_LINE=""
declare USING_DOCKER="no"
declare USING_CLION="no"
declare DOCKER_OS_TYPE="${DOCKER_BUILD_OS_TYPE}"
declare DOCKER_OS_TAG="${DOCKER_BUILD_OS_TAG}"
declare DOCKER_TAG="${DOCKER_BUILD_TAG}"
declare EXTRA_ARGS=""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    -b|--build)
      declare -r BUILD_DIRNAME=${2}
      GOOM_VAR_CMD_LINE="${GOOM_VAR_CMD_LINE} --build ${BUILD_DIRNAME}"
      shift # past argument
      shift # past value
      ;;
    -c|--compiler)
      COMPILER=${2}
      GOOM_VAR_CMD_LINE="${GOOM_VAR_CMD_LINE} --compiler ${COMPILER}"
      shift # past argument
      shift # past value
      ;;
    --build-type)
      BUILD_TYPE=${2}
      GOOM_VAR_CMD_LINE="${GOOM_VAR_CMD_LINE} --build-type ${BUILD_TYPE}"
      shift # past argument
      shift # past value
      ;;
    -s|--suffix)
      declare -r BUILD_DIR_SUFFIX=${2}
      GOOM_VAR_CMD_LINE="${GOOM_VAR_CMD_LINE} --suffix ${BUILD_DIR_SUFFIX}"
      shift # past argument
      shift # past value
      ;;
    --docker)
      USING_DOCKER="yes"
      GOOM_VAR_CMD_LINE="${GOOM_VAR_CMD_LINE} --docker"
      shift # past argument
      ;;
    --docker-os-type)
      DOCKER_OS_TYPE=${2}
      GOOM_VAR_CMD_LINE="${GOOM_VAR_CMD_LINE} --docker-os-type ${DOCKER_OS_TYPE}"
      shift # past argument
      shift # past value
      ;;
    --docker-os-tag)
      DOCKER_OS_TAG=${2}
      GOOM_VAR_CMD_LINE="${GOOM_VAR_CMD_LINE} --docker-os-tag ${DOCKER_OS_TAG}"
      shift # past argument
      shift # past value
      ;;
    --clion)
      USING_CLION="yes"
      GOOM_VAR_CMD_LINE="${GOOM_VAR_CMD_LINE} --clion"
      shift # past argument
      ;;
    *)
      EXTRA_ARGS="${EXTRA_ARGS}${key} "
      shift # past argument
      ;;
    *)
  esac
done

set -- ${EXTRA_ARGS}
unset EXTRA_ARGS


# Compilers
if [[ "${COMPILER:-}" == "" ]]; then
  echo "ERROR: 'COMPILER' must be specified."
  echo
  exit 1
fi
if [[ "${COMPILER}" == "clang" ]]; then
  declare -r COMPILER_NO_VER=clang
  declare -r C_COMPILER=clang
  declare -r CPP_COMPILER=clang++
  declare -r COMPILER_VERSION=
elif [[ "${COMPILER}" == "clang-18" ]]; then
  declare -r COMPILER_NO_VER=clang
  declare -r C_COMPILER=clang-18
  declare -r CPP_COMPILER=clang-18
  declare -r COMPILER_VERSION=18
elif [[ "${COMPILER}" == "clang-19" ]]; then
  declare -r COMPILER_NO_VER=clang
  declare -r C_COMPILER=clang-19
  declare -r CPP_COMPILER=clang-19
  declare -r COMPILER_VERSION=19
else
  echo "ERROR: Unknown compiler \"${COMPILER}\"."
  echo
  exit 1
fi

# Build type
if [[ "${BUILD_TYPE:-}" == "" ]]; then
  echo "ERROR: 'BUILD_TYPE' must be specified."
  echo
  exit 1
fi
if [[ "${BUILD_TYPE}" == "Debug" ]]; then
  declare -r C_BUILD_TYPE=Debug
elif [[ "${BUILD_TYPE}" == "Release" ]]; then
  declare -r C_BUILD_TYPE=Release
elif [[ "${BUILD_TYPE}" == "RelWithDebInfo" ]]; then
  declare -r C_BUILD_TYPE=RelWithDebInfo
else
  echo "ERROR: Unknown build type \"${BUILD_TYPE}\"."
  echo
  exit 1
fi

# Suffix
if [[ "${BUILD_DIR_SUFFIX:-}" == "" ]]; then
  echo "ERROR: 'BUILD_DIR_SUFFIX' must be specified."
  echo
  exit 1
fi

# Docker
if [[ "${USING_DOCKER}" == "no" ]]; then
  declare DOCKER_PREFIX=""
else
  declare DOCKER_PREFIX="docker-"
fi

declare -r HOST_TIME_ZONE=$(cat /etc/timezone)
declare -r DOCKER_BUILD_IMAGE="$(get_docker_build_image ${DOCKER_OS_TYPE} ${DOCKER_OS_TAG} ${DOCKER_TAG} ${COMPILER_NO_VER} ${COMPILER_VERSION})"

# Clion
if [[ "${USING_CLION}" == "no" ]]; then
  declare CLION_PREFIX=""
else
  declare CLION_PREFIX="clion-"
fi

# Build directory
if [[ "${BUILD_DIRNAME:-}" == "" ]]; then
  declare -r BUILD_DIRNAME=build-${CLION_PREFIX}${DOCKER_PREFIX}${C_COMPILER}-${C_BUILD_TYPE}-${BUILD_DIR_SUFFIX}
fi
if [[ ${BUILD_DIRNAME} != build* ]]; then
  echo "ERROR: Build dirname must start with \"build\". Not this: BUILD_DIRNAME = \"${BUILD_DIRNAME}\""
  exit 1
fi

if [[ "${OVERRIDE_BUILD_PARENT_DIR:-}" != "" ]]; then
  declare -r BUILD_DIR=${OVERRIDE_BUILD_PARENT_DIR}/${BUILD_DIRNAME}
else
  declare -r BUILD_DIR=${GOOM_MAIN_ROOT_DIR}/${BUILD_DIRNAME}
fi


unset DOCKER_PREFIX
unset CLION_PREFIX
