set(soundkonverter_replaygain_vorbisgain_SRCS
   soundkonverter_replaygain_vorbisgain.cpp
)

kcoreaddons_add_plugin(soundkonverter_replaygain_vorbisgain
    SOURCES ${soundkonverter_replaygain_vorbisgain_SRCS}
    INSTALL_NAMESPACE soundkonverterplugins/replaygain)

target_link_libraries(soundkonverter_replaygain_vorbisgain KF6::WidgetsAddons Qt6::Xml soundkonvertercore)
