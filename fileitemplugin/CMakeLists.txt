
kcoreaddons_add_plugin(
    plasmavaultfileitemaction
    SOURCES plasmavaultfileitemaction.cpp
    JSON plasmavaultfileitemaction.json
    INSTALL_NAMESPACE "kf5/kfileitemaction")

target_link_libraries(plasmavaultfileitemaction
    KF5::I18n
    KF5::KIOWidgets)
