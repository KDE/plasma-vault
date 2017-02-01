# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (PlasmaVaultTest)

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets Test Quick QuickWidgets)

include_directories (
   ${CMAKE_SOURCE_DIR}/src/
   )

set (
   PlasmaVaultTest_SRCS
   main.cpp
   window.cpp
   )

qt5_wrap_ui(
   PlasmaVaultTest_SRCS
   window.ui
   )

if (NOT WIN32)

   add_executable (
      PlasmaVaultTest
      ${PlasmaVaultTest_SRCS}
      )

   target_link_libraries (
      PlasmaVaultTest
      Qt5::Core
      Qt5::Gui
      Qt5::Widgets
      KF5::PlasmaVault
      )

endif ()
