add_library(openvfs SHARED
        xattr.cpp
        openvfs.cpp
)
target_include_directories(openvfs PUBLIC "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}>" "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>" $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)
target_link_libraries(openvfs PRIVATE $<BUILD_INTERFACE:nlohmann_json::nlohmann_json>)
set_target_properties(openvfs PROPERTIES EXPORT_NAME LibOpenVFS)
generate_export_header(openvfs
        EXPORT_MACRO_NAME OPENVFS_EXPORT
        EXPORT_FILE_NAME openvfs_export.h
)


install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/../openvfs/version.h
            ${CMAKE_CURRENT_BINARY_DIR}/openvfs_export.h
            openvfconstants.h
            openvfs.h
        DESTINATION
            ${KDE_INSTALL_INCLUDEDIR}/openvfs
)
install(TARGETS openvfs
        EXPORT OpenVFSConfig ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)
