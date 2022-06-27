# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

cmake_minimum_required(VERSION 3.16)

project (PlasmaVault)

set(PROJECT_VERSION "5.25.80")
set(PROJECT_VERSION_MAJOR 5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (PLASMAVAULT_VERSION "0.1")

# We don't build in-source
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message (
      FATAL_ERROR
      "plasmavault requires an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there."
   )
endif ()

# Extra CMake stuff
include (FeatureSummary)

set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.94")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

find_package (ECM ${KF5_MIN_VERSION}  NO_MODULE)
set_package_properties (ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary (WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

include (KDEInstallDirs)
include (KDECMakeSettings)
include (KDECompilerSettings)
include (GenerateExportHeader)
include (ECMGenerateHeaders)
include(KDEClangFormat)
include(KDEGitCommitHooks)
# Qt
set (CMAKE_AUTOMOC ON)
find_package (
   Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED
   COMPONENTS
      Core
      DBus
      Quick
      Widgets
   )

find_package (
   KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
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
find_package (KF5NetworkManagerQt ${KF5_MIN_VERSION})
if (KF5NetworkManagerQt_FOUND)
    set (HAVE_NETWORKMANAGER TRUE)
endif()
find_package (KSysGuard REQUIRED)

include_directories (
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/common
   )

# Basic includes
include (CPack)

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
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

