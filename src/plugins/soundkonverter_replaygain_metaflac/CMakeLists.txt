set(soundkonverter_replaygain_metaflac_SRCS
   soundkonverter_replaygain_metaflac.cpp
)

kcoreaddons_add_plugin(soundkonverter_replaygain_metaflac
    SOURCES ${soundkonverter_replaygain_metaflac_SRCS}
    INSTALL_NAMESPACE soundkonverterplugins/replaygain)

target_link_libraries(soundkonverter_replaygain_metaflac KF6::WidgetsAddons Qt6::Xml soundkonvertercore)
