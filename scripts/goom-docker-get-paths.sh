source "${GOOM_MAIN_ROOT_DIR}/dockerize-kodi-goom/goom-image-funcs.sh"


declare KODI_IMAGE_OS_TYPE="ubuntu"
declare KODI_IMAGE_OS_TAG="23.04"
declare GOOM_DOCKER_PATHS_CMD_LINE=""
declare EXTRA_ARGS=""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    --image-os)
      KODI_IMAGE_OS_TYPE=$2
      shift # past argument
      shift # past value
      GOOM_DOCKER_PATHS_CMD_LINE="${GOOM_DOCKER_PATHS_CMD_LINE} --image-os ${KODI_IMAGE_OS_TYPE}"
      ;;
    --image-os-version)
      KODI_IMAGE_OS_TAG=$2
      shift # past argument
      shift # past value
      GOOM_DOCKER_PATHS_CMD_LINE="${GOOM_DOCKER_PATHS_CMD_LINE} --image-os-version ${KODI_IMAGE_OS_TAG}"
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


declare -r DOCKERIZE_KODI_DIR=${GOOM_MAIN_ROOT_DIR}/dockerize-kodi-goom
if [[ ! -d "${DOCKERIZE_KODI_DIR}" ]]; then
  echo "ERROR: Could not find dockerize kodi directory \"${DOCKERIZE_KODI_DIR}\"."
  exit 1
fi

declare -r KODI_DOCKER_FILES_DIR=${GOOM_MAIN_ROOT_DIR}/dockerize-kodi-goom/files
if [[ ! -d "${KODI_DOCKER_FILES_DIR}" ]]; then
  echo "ERROR: Could not find kodi docker files directory \"${KODI_DOCKER_FILES_DIR}\"."
  exit 1
fi
declare -r KODI_DOCKER_RESOURCES_DIR=${KODI_DOCKER_FILES_DIR}/resources
if [[ ! -d "${KODI_DOCKER_RESOURCES_DIR}" ]]; then
  echo "ERROR: Could not find kodi docker resources directory \"${KODI_DOCKER_RESOURCES_DIR}\"."
  exit 1
fi


declare -r ADDON_XML="${KODI_BUILD_SHARE_DIR}/addon.xml"
if [[ ! -f "${ADDON_XML}" ]]; then
  echo "ERROR: Could not find add-on xml file \"${ADDON_XML}\"."
  exit 1
fi
declare -r GOOM_VERSION=$(sed -n 's#.*version="\([0-9][0-9].*[\.0-9]\)".*#\1#p' < "${ADDON_XML}")

if [[ "${GOOM_VERSION:0:2}" == "19" ]]; then
  declare -r KODI_VERSION=matrix
elif [[ "${GOOM_VERSION:0:2}" == "20" ]]; then
  declare -r KODI_VERSION=nexus
else
  echo "ERROR: Unknown Goom version: \"${GOOM_VERSION}\" (from ${ADDON_XML})."
  exit 1
fi

declare -r KODI_GOOM_IMAGE="$(get_kodi_goom_image_name ${KODI_IMAGE_OS_TYPE} ${KODI_IMAGE_OS_TAG} ${KODI_VERSION})"
declare -r KODI_CONTAINER_HOME_DIR="${HOME}/docker/kodi-${KODI_VERSION}"
declare -r KODI_CONTAINER_NAME="kodi_goom_dev"
declare -r MUSIC_SHARE="/mnt/Music"

declare -r REMOTE_KODI_GOOM_DIR="${HOME}/Prj/github/xbmc/visualization.goom-pp"
declare -r REMOTE_KODI_BUILD_DIR="${REMOTE_KODI_GOOM_DIR}/dockerize-kodi-goom"
