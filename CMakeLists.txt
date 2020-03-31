# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

cmake_minimum_required (VERSION 2.8.12)

project (PlasmaVault)

set(PROJECT_VERSION "5.18.4")
set(PROJECT_VERSION_MAJOR 5)

set (PLASMAVAULT_VERSION "0.1")

set (QT_MIN_VERSION 5.12.0)

# We don't build in-source
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message (
      FATAL_ERROR
      "plasmavault requires an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there."
   )
endif ()

# Extra CMake stuff
include (FeatureSummary)

# KDE Frameworks
set (KF5_DEP_VERSION "5.17.0") # handled by release scripts

find_package (ECM ${KF5_DEP_VERSION}  NO_MODULE)
set_package_properties (ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary (WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include (KDEInstallDirs)
include (KDECMakeSettings)
include (KDECompilerSettings)
include (GenerateExportHeader)
include (ECMGenerateHeaders)
include(KDEClangFormat)
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
   add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)
   add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054200)
endif()
# Qt
set (CMAKE_AUTOMOC ON)
find_package (
   Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED
   COMPONENTS
      Core
      DBus
      Quick
      Widgets
   )

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
   )
find_package (KF5NetworkManagerQt ${KF5_DEP_VERSION})
if (KF5NetworkManagerQt_FOUND)
    set (HAVE_NETWORKMANAGER TRUE)
endif()
find_package (KF5SysGuard REQUIRED)

include_directories (
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/common
   )

# Basic includes
include (CPack)

include (ECMPackageConfigHelpers)
include (ECMSetupVersion)

add_definitions (-DQT_NO_URL_CAST_FROM_STRING)
add_definitions (-DQT_NO_CAST_TO_ASCII)
add_definitions (-DTRANSLATION_DOMAIN=\"plasmavault-kde\")
add_definitions (-fexceptions)
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ki18n_install (po)
endif ()

# libPlasmaVault

add_subdirectory (kded)
add_subdirectory (plasma)
add_subdirectory (icons)
add_subdirectory (fileitemplugin)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

