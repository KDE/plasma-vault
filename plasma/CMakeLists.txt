
set (
   plasmavault_SRCS

   vaultapplet.cpp
   vaultsmodel.cpp

   ../common/vaultinfo.cpp
   )

set_source_files_properties (
   ../common/org.kde.plasmavault.xml
   PROPERTIES
   INCLUDE ../common/vaultinfo.h
   )

qt5_add_dbus_interface (
   plasmavault_SRCS
   ../common/org.kde.plasmavault.xml
   plasmavault_interface
   )

add_library (
    plasma_applet_vault
    MODULE ${plasmavault_SRCS}
    )

kcoreaddons_desktop_to_json (
   plasma_applet_vault
   package/metadata.desktop
   SERVICE_TYPES plasma-applet.desktop
   )

target_link_libraries (
   plasma_applet_vault

   Qt5::Gui
   Qt5::DBus

   KF5::Activities
   KF5::I18n
   KF5::Plasma
   )

install (
   TARGETS plasma_applet_vault
   DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets
   )

plasma_install_package (package org.kde.plasma.vault)

