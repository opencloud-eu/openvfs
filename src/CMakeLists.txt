include(CMakePackageConfigHelpers)
include(ECMMarkNonGuiExecutable)
include(ECMSetupVersion)

add_subdirectory(openvfs)
add_subdirectory(openvfsfuse)
add_subdirectory(vfsstat)


ecm_setup_version(PROJECT
        VARIABLE_PREFIX OPENVFS
        VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/openvfs/version.h
        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/OpenVFSConfigVersion.cmake"
)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/OpenVFSConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/OpenVFSConfig.cmake"
        INSTALL_DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/openvfs
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/OpenVFSConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/OpenVFSConfigVersion.cmake"
        DESTINATION
        ${KDE_INSTALL_CMAKEPACKAGEDIR}/openvfs
)
install(EXPORT OpenVFSConfig
        DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/openvfs
        FILE OpenVfsTargets.cmake
        NAMESPACE OpenVFS::
)
