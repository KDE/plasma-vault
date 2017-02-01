prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${BIN_INSTALL_DIR}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}

Name: libPlasmaVault
Description: libPlasmaVault provides Qt interface for encfs
URL: http://www.kde.org
Requires:
Version: ${PLASMAVAULT_LIB_VERSION_STRING}
Libs: -L${LIB_INSTALL_DIR} -lKF5PlasmaVault
Cflags: -I${INCLUDE_INSTALL_DIR}
