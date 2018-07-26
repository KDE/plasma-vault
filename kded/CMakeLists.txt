configure_file (config-plasma-vault.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-plasma-vault.h)

include_directories (
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/engine
   )

set (
   kded_plasmavault_SRCS
   service.cpp

   engine/vault.cpp
   engine/backend_p.cpp
   engine/fusebackend_p.cpp
   engine/types.cpp
   engine/commandresult.cpp

   engine/backends/encfs/encfsbackend.cpp
   engine/backends/cryfs/cryfsbackend.cpp

   ui/dialogdsl.cpp
   ui/activitieslinkingwidget.cpp
   ui/backendchooserwidget.cpp
   ui/cryfscypherchooserwidget.cpp
   ui/directorypairchooserwidget.cpp
   ui/directorychooserwidget.cpp
   ui/namechooserwidget.cpp
   ui/noticewidget.cpp
   ui/passwordchooserwidget.cpp
   ui/offlineonlywidget.cpp
   ui/vaultdeletionwidget.cpp

   ui/vaultcreationwizard.cpp
   ui/vaultimportingwizard.cpp
   ui/vaultconfigurationwizard.cpp
   ui/mountdialog.cpp

   ../common/vaultinfo.cpp
   )

ki18n_wrap_ui (
   kded_plasmavault_SRCS
   ui/activitieslinkingwidget.ui
   ui/backendchooserwidget.ui
   ui/cryfscypherchooserwidget.ui
   ui/directorypairchooserwidget.ui
   ui/directorychooserwidget.ui
   ui/namechooserwidget.ui
   ui/noticewidget.ui
   ui/passwordchooserwidget.ui
   ui/offlineonlywidget.ui
   ui/vaultdeletionwidget.ui

   ui/vaultcreationwizard.ui
   ui/vaultimportingwizard.ui
   ui/vaultconfigurationwizard.ui
   ui/mountdialog.ui
   )

add_library (
   kded_plasmavault
   MODULE ${kded_plasmavault_SRCS}
   )

set_target_properties (
   kded_plasmavault
   PROPERTIES OUTPUT_NAME plasmavault
   )

kcoreaddons_desktop_to_json (
   kded_plasmavault plasmavault.desktop
   )

target_link_libraries (
   kded_plasmavault

   Qt5::Core
   Qt5::DBus
   Qt5::Widgets

   KF5::Activities
   KF5::ConfigCore
   KF5::ConfigWidgets
   KF5::CoreAddons
   KF5::DBusAddons
   KF5::I18n
   KF5::KIOCore
   KF5::KIOWidgets
   KF5::SysGuard
   KF5::WidgetsAddons
   KF5::ProcessCore
   )
if (HAVE_NETWORKMANAGER)
    target_link_libraries (
       kded_plasmavault
       KF5::NetworkManagerQt
    )
endif ()


install (
    TARGETS kded_plasmavault DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kded)

