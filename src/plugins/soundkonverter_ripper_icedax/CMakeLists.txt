set(soundkonverter_ripper_icedax_SRCS
   soundkonverter_ripper_icedax.cpp
)

kcoreaddons_add_plugin(soundkonverter_ripper_icedax
    SOURCES ${soundkonverter_ripper_icedax_SRCS}
    INSTALL_NAMESPACE soundkonverterplugins/ripper)

target_link_libraries(soundkonverter_ripper_icedax KF6::WidgetsAddons Qt6::Xml soundkonvertercore)
