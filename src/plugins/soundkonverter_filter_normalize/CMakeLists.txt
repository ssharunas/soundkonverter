set(soundkonverter_filter_normalize_SRCS
   soundkonverter_filter_normalize.cpp
   normalizefilteroptions.cpp
   normalizefilterwidget.cpp
)

kcoreaddons_add_plugin(soundkonverter_filter_normalize
    SOURCES ${soundkonverter_filter_normalize_SRCS}
    INSTALL_NAMESPACE soundkonverterplugins/filters)

target_link_libraries(soundkonverter_filter_normalize KF6::WidgetsAddons Qt6::Xml soundkonvertercore)
