
set (
   plasmavault_helper_SRCS
   tombhelper.cpp
   )

add_executable (
   plasmavault_helper
   ${plasmavault_helper_SRCS}
   )

target_link_libraries (
   plasmavault_helper
   KF5::Auth
   )

install (
   TARGETS plasmavault_helper
   DESTINATION ${KAUTH_HELPER_INSTALL_DIR}
   )

kauth_install_helper_files (plasmavault_helper org.kde.plasma.vault.tomb root)
kauth_install_actions (org.kde.plasma.vault.tomb plasmavault_tomb.actions)

