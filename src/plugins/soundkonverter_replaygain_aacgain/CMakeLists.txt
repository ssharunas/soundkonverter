set(soundkonverter_replaygain_aacgain_SRCS
   soundkonverter_replaygain_aacgain.cpp
)

kcoreaddons_add_plugin(soundkonverter_replaygain_aacgain
    SOURCES ${soundkonverter_replaygain_aacgain_SRCS}
    INSTALL_NAMESPACE soundkonverterplugins/replaygain)

target_link_libraries(soundkonverter_replaygain_aacgain KF6::WidgetsAddons Qt6::Xml soundkonvertercore)
