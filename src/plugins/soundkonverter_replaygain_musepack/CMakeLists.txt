set(soundkonverter_replaygain_musepackgain_SRCS
   soundkonverter_replaygain_musepackgain.cpp
)

kcoreaddons_add_plugin(soundkonverter_replaygain_musepackgain
    SOURCES ${soundkonverter_replaygain_musepackgain_SRCS}
    INSTALL_NAMESPACE soundkonverterplugins/replaygain)

target_link_libraries(soundkonverter_replaygain_musepackgain KF6::WidgetsAddons Qt6::Xml soundkonvertercore)
