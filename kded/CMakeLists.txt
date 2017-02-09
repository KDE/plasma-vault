
set(kded_plasmavault_SRCS
    service.cpp

    ui/dialogdsl.cpp
    ui/activitieslinkingwidget.cpp
    ui/backendchooserwidget.cpp
    ui/cryfscypherchooserwidget.cpp
    ui/directorypairchooserwidget.cpp
    ui/noticewidget.cpp
    ui/passwordchooserwidget.cpp
    ui/vaultcreationwizard.cpp
)

ki18n_wrap_ui(kded_plasmavault_SRCS
    ui/activitieslinkingwidget.ui
    ui/backendchooserwidget.ui
    ui/cryfscypherchooserwidget.ui
    ui/directorypairchooserwidget.ui
    ui/noticewidget.ui
    ui/passwordchooserwidget.ui
    ui/vaultcreationwizard.ui
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
    PlasmaVault
)


install(TARGETS kded_plasmavault DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kded)

