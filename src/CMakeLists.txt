# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

set (
   PlasmaVault_LIB_SRCS

   vault.cpp
   backend_p.cpp
   common.cpp
   commandresult.cpp

   backends/encfs/encfsbackend.cpp
   )

add_library (
   PlasmaVault SHARED
   ${PlasmaVault_LIB_SRCS}
   )

set_target_properties (
   PlasmaVault
   PROPERTIES
   VERSION ${PLASMAVAULT_VERSION_STRING}
   SOVERSION ${PLASMAVAULT_SOVERSION}
   EXPORT_NAME PlasmaVault
   )

target_link_libraries (
   PlasmaVault
   PUBLIC
      Qt5::Core
      Qt5::DBus
   PRIVATE
      KF5::ConfigCore
      KF5::I18n
      KF5::KIOCore
   )

target_include_directories (
   PlasmaVault
   INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/PlasmaVault>"
   )

# install
generate_export_header (PlasmaVault BASE_NAME PlasmaVault)

# ecm_generate_headers (
#    PlasmaVault_CamelCase_HEADERS
#    HEADER_NAMES
#    Common
#    Vault
#    CommandResult
#    PREFIX PlasmaVault
#    REQUIRED_HEADERS PlasmaVault_HEADERS
#    )

# install (
#    FILES ${PlasmaVault_CamelCase_HEADERS}
#    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/PlasmaVault/PlasmaVault
#    COMPONENT Devel
#    )

# install (
#    FILES ${PlasmaVault_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/plasmavault_export.h
#    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/PlasmaVault/plasmavault
#    COMPONENT Devel
#    )

install (
   TARGETS PlasmaVault
   EXPORT PlasmaVaultLibraryTargets
   ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
   )

# if (NOT WIN32)
#    configure_file (
#       ${CMAKE_CURRENT_SOURCE_DIR}/libPlasmaVault.pc.cmake
#       ${CMAKE_CURRENT_BINARY_DIR}/libPlasmaVault.pc
#       )
#    install (
#       FILES ${CMAKE_CURRENT_BINARY_DIR}/libPlasmaVault.pc
#       DESTINATION ${KDE_INSTALL_LIBDIR}/pkgconfig
#       )
# endif ()

# include (ECMGeneratePriFile)
# ecm_generate_pri_file (
#    BASE_NAME PlasmaVault
#    LIB_NAME KF5PlasmaVault
#    DEPS "KDBusAddons"
#    FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/PlasmaVault
#    )
# install (
#    FILES ${PRI_FILENAME}
#    DESTINATION ${ECM_MKSPECS_INSTALL_DIR}
#    )

