set(soundkonverter_replaygain_wvgain_SRCS
   soundkonverter_replaygain_wvgain.cpp
)

kcoreaddons_add_plugin(soundkonverter_replaygain_wvgain
    SOURCES ${soundkonverter_replaygain_wvgain_SRCS}
    INSTALL_NAMESPACE soundkonverterplugins/replaygain)

target_link_libraries(soundkonverter_replaygain_wvgain KF6::WidgetsAddons Qt6::Xml soundkonvertercore)
