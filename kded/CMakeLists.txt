
set(kded_plasmavault_SRCS
    service.cpp
    encfscreatedialog.cpp
)

ki18n_wrap_ui(kded_plasmavault_SRCS
    encfscreatedialog.ui
)

add_library(kded_plasmavault MODULE ${kded_plasmavault_SRCS})
set_target_properties(kded_plasmavault PROPERTIES OUTPUT_NAME plasmavault)
kcoreaddons_desktop_to_json(kded_plasmavault plasmavault.desktop)

target_link_libraries(kded_plasmavault
    Qt5::Widgets
    Qt5::DBus
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::WidgetsAddons
    KF5::ConfigWidgets
    KF5::KIOWidgets
    KF5::I18n
    KF5::Activities
    KF5::PlasmaVault
)


install(TARGETS kded_plasmavault DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kded)

