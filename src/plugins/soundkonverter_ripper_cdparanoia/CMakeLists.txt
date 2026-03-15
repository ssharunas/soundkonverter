set(soundkonverter_ripper_cdparanoia_SRCS
   soundkonverter_ripper_cdparanoia.cpp
)

kcoreaddons_add_plugin(soundkonverter_ripper_cdparanoia
    SOURCES ${soundkonverter_ripper_cdparanoia_SRCS}
    INSTALL_NAMESPACE soundkonverterplugins/ripper)

target_link_libraries(soundkonverter_ripper_cdparanoia KF6::WidgetsAddons Qt6::Xml soundkonvertercore)
