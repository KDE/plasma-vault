
set (
   plasmavault_SRCS
   applet.cpp
   vaultsmodel.cpp
   )

set_source_files_properties (
   ../kded/org.kde.plasmavault.xml
   PROPERTIES
   INCLUDE ../src/vault.h
   )

qt5_add_dbus_interface (
   plasmavault_SRCS
   ../kded/org.kde.plasmavault.xml
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
   KF5::Plasma
   KF5::I18n
   PlasmaVault
   )

install (
   TARGETS plasma_applet_plasmavault
   DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets
   )

plasma_install_package (package org.kde.plasmavault)

