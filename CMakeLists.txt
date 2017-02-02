# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

cmake_minimum_required (VERSION 2.8.11)

project (PlasmaVault)
set (PLASMAVAULT_VERSION "0.1")

set (REQUIRED_QT_VERSION 5.3.0)

# We don't build in-source
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message (
      FATAL_ERROR
      "plasmavault requires an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there."
   )
endif ()

# Extra CMake stuff
include (FeatureSummary)
find_package (ECM 5.17.0  NO_MODULE)
set_package_properties (ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary (WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include (KDEInstallDirs)
include (KDECMakeSettings)
include (KDECompilerSettings)
include (GenerateExportHeader)
include (ECMGenerateHeaders)

# Qt
set (CMAKE_AUTOMOC ON)
find_package (
   Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED COMPONENTS
   Core
   DBus
   Quick
   Widgets
   )

# KDE Frameworks
set(KF5_VERSION "5.18.0") # handled by release scripts
set(KF5_DEP_VERSION "5.17.0") # handled by release scripts

find_package (
   KF5 REQUIRED COMPONENTS
   Activities
   AsynQt
   Config
   ConfigWidgets
   CoreAddons
   DBusAddons
   I18n
   KIO
   KIO
   Plasma
   WidgetsAddons
   )


include_directories (
   ${AsynQt_INCLUDE_DIRS}
   ${CMAKE_CURRENT_SOURCE_DIR}/src
   ${CMAKE_CURRENT_BINARY_DIR}/src
   )

# Basic includes
include (CPack)

include (ECMPackageConfigHelpers)
include (ECMSetupVersion)

add_definitions (-DTRANSLATION_DOMAIN=\"plasmavault\")
add_definitions (-fexceptions)
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ki18n_install (po)
endif ()

# libPlasmaVault

ecm_setup_version (
   ${PLASMAVAULT_VERSION}
   VARIABLE_PREFIX PLASMAVAULT
   VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/plasmavault_version.h"
   PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/PlasmaVaultConfigVersion.cmake"
   SOVERSION 0
   )

set (CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/PlasmaVault")

install (
   EXPORT PlasmaVaultLibraryTargets
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   FILE PlasmaVaultLibraryTargets.cmake
   )

ecm_configure_package_config_file (
   "${CMAKE_CURRENT_SOURCE_DIR}/PlasmaVaultConfig.cmake.in"
   "${CMAKE_CURRENT_BINARY_DIR}/PlasmaVaultConfig.cmake"
   INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
   PATH_VARS  KF5_INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
   )

install (
   FILES "${CMAKE_CURRENT_BINARY_DIR}/PlasmaVaultConfig.cmake"
         "${CMAKE_CURRENT_BINARY_DIR}/PlasmaVaultConfigVersion.cmake"
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   COMPONENT Devel
   )

install (
   FILES ${CMAKE_CURRENT_BINARY_DIR}/plasmavault_version.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
   )

add_subdirectory (src)
add_subdirectory (kded)
add_subdirectory (plasma)

add_subdirectory (autotests)
add_subdirectory (tests)

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

