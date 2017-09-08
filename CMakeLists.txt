# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

cmake_minimum_required (VERSION 2.8.12)

project (PlasmaVault)

set(PROJECT_VERSION "5.10.90")
set(PROJECT_VERSION_MAJOR 5)

set (PLASMAVAULT_VERSION "0.1")

set (REQUIRED_QT_VERSION 5.7.0)

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
   Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED
   COMPONENTS
      Core
      DBus
      Quick
      Widgets
   )

# KDE Frameworks
set (KF5_DEP_VERSION "5.17.0") # handled by release scripts

find_package (
   KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS
   Activities
   Config
   ConfigWidgets
   CoreAddons
   DBusAddons
   I18n
   KIO
   Plasma
   WidgetsAddons
   NetworkManagerQt
   )
find_package (KF5SysGuard REQUIRED)

include_directories (
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/common
   )

# Basic includes
include (CPack)

include (ECMPackageConfigHelpers)
include (ECMSetupVersion)

add_definitions (-DTRANSLATION_DOMAIN=\"plasmavault-kde\")
add_definitions (-fexceptions)
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ki18n_install (po)
endif ()

# libPlasmaVault

add_subdirectory (kded)
add_subdirectory (plasma)
add_subdirectory (icons)

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

