set(soundkonverter_filter_sox_SRCS
   soundkonverter_filter_sox.cpp
   soxfilteroptions.cpp
   soxcodecwidget.cpp
   soxfilterwidget.cpp
   soxeffectwidget.cpp
)

kcoreaddons_add_plugin(soundkonverter_filter_sox
    SOURCES ${soundkonverter_filter_sox_SRCS}
    INSTALL_NAMESPACE soundkonverterplugins/filters)

target_link_libraries(soundkonverter_filter_sox KF6::WidgetsAddons Qt6::Xml soundkonvertercore)
