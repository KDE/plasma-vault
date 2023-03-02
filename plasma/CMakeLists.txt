
set (
   plasmavault_SRCS

   vaultapplet.cpp
   vaultapplet.h
   vaultsmodel.cpp
   vaultsmodel.h
   vaultsmodel_p.h

   ../common/vaultinfo.cpp
   ../common/vaultinfo.h
   )

set_source_files_properties (
   ../common/org.kde.plasmavault.xml
   PROPERTIES
   INCLUDE ../common/vaultinfo.h
   )

qt_add_dbus_interface (
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

   Qt::Gui
   Qt::DBus

   KF6::Activities
   KF6::I18n
   KF6::Plasma
   )

install (
   TARGETS plasma_applet_vault
   DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets
   )

plasma_install_package (package org.kde.plasma.vault)

