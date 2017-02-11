
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

add_library (plasma_applet_plasmavault MODULE ${plasmavault_SRCS})

kcoreaddons_desktop_to_json (
   plasma_applet_plasmavault
   package/metadata.desktop
   SERVICE_TYPES plasma-applet.desktop
   )

target_link_libraries (
   plasma_applet_plasmavault

   Qt5::Gui
   Qt5::DBus

   KF5::Plasma
   KF5::I18n
   )

install (
   TARGETS plasma_applet_plasmavault
   DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets
   )

plasma_install_package (package org.kde.plasmavault)

