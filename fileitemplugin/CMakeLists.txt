
kcoreaddons_add_plugin(
    plasmavaultfileitemaction
    SOURCES plasmavaultfileitemaction.cpp plasmavaultfileitemaction.h
    INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/kfileitemaction")

target_link_libraries(plasmavaultfileitemaction
    KF5::I18n
    KF5::KIOWidgets)
