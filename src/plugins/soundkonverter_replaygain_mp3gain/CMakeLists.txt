set(soundkonverter_replaygain_mp3gain_SRCS
   soundkonverter_replaygain_mp3gain.cpp
)

kcoreaddons_add_plugin(soundkonverter_replaygain_mp3gain
    SOURCES ${soundkonverter_replaygain_mp3gain_SRCS}
    INSTALL_NAMESPACE soundkonverterplugins/replaygain)

target_link_libraries(soundkonverter_replaygain_mp3gain KF6::WidgetsAddons Qt6::Xml soundkonvertercore)
