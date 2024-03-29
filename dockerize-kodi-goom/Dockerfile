ARG OS_TYPE
ARG OS_TAG

FROM ${OS_TYPE}:${OS_TAG}

ARG DEBIAN_FRONTEND=noninteractive

# Install Kodi
#
# NOTE: For packages, we need 'kodi-eventclients-kodi-send' to shut down
#       Kodi gracefully when it's container terminates.
#       'psmisc' is needed for 'killall'.

ARG KODI_PPA=""
ARG END_OF_LIFE="no"

ARG KODI_ESSENTIAL_PACKAGES=" \
  ca-certificates             \
  kodi                        \
  kodi-inputstream-adaptive   \
  kodi-repository-kodi        \
  locales                     \
  pulseaudio                  \
  tzdata                      \
  va-driver-all               \
  psmisc"

ARG KODI_EXTRA_PACKAGES="     \
  kodi-eventclients-kodi-send \
  less                        \
  gdb"

ARG KODI_PACKAGES="${KODI_ESSENTIAL_PACKAGES} ${KODI_EXTRA_PACKAGES}"

RUN if [ "${END_OF_LIFE}" = "yes" ] ; then                                   \
      sed -i -e 's/archive.ubuntu.com\|security.ubuntu.com/old-releases.ubuntu.com/g' /etc/apt/sources.list ; \
    fi                                                                    && \
    apt-get update                                                        && \
    apt-get install -y gpg-agent                                          && \
    apt-get install -y --no-install-recommends software-properties-common && \
    if [ "${KODI_PPA}" != "" ] ; then                                        \
      add-apt-repository -y ${KODI_PPA} ;                                    \
    fi                                                                    && \
    apt-get install -y --no-install-recommends ${KODI_PACKAGES}           && \
                                                                             \
    apt-get -y purge software-properties-common                           && \
    apt-get -y --purge autoremove                                         && \
    rm -rf /var/lib/apt/lists/*                                           && \
    apt-get clean -y


# Install lirc
ARG LIRC_SEED="lirc/remote select Windows Media Center Transceivers/Remotes (all)"
ARG LIRC_SEED_FILE=/etc/lirc/lirc_mce.seed

RUN mkdir -p /etc/lirc                                                    && \
    echo "lirc ${LIRC_SEED}" > "${LIRC_SEED_FILE}"                        && \
    echo "lirc lirc/transmitter select None" >> "${LIRC_SEED_FILE}"       && \
    debconf-set-selections "${LIRC_SEED_FILE}"                            && \
    apt-get update                                                        && \
    apt-get install -y lirc                                               && \
    rm -f "${LIRC_SEED_FILE}"                                             && \
    mkdir -p /var/run/lirc/                                               && \
    ln -sf /dev/lircd /var/run/lirc/lircd                                 && \
    apt-get clean -y


# Copy the already built goom files to Kodi add-on directories.
ARG GOOM_VERSION
ARG KODI_GOOM_LIB_DIR
ARG KODI_GOOM_ADDON_DIR
RUN rm -rf "${KODI_GOOM_ADDON_DIR}/*"
COPY "files/visualization.goom-pp.so.${GOOM_VERSION}" "${KODI_GOOM_LIB_DIR}/visualization.goom-pp.so.${GOOM_VERSION}"
COPY "files/addon.xml"                             "${KODI_GOOM_ADDON_DIR}/"
COPY "files/resources"                             "${KODI_GOOM_ADDON_DIR}/resources/"
COPY "files/irexec.lircrc"                         "/etc/lirc/irexec.lircrc"


COPY entrypoint.sh /usr/local/bin
RUN chmod +x /usr/local/bin/entrypoint.sh
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
