add_executable(openvfsfuse
        main.cpp
        openvfsfuse.cpp
        socketthread.cpp
        sharedmap.cpp

        strtools.cpp
)
set_target_properties(openvfsfuse PROPERTIES EXPORT_NAME OpenVFS)
ecm_mark_nongui_executable(openvfsfuse)
target_link_libraries(openvfsfuse PRIVATE openvfs nlohmann_json::nlohmann_json FUSE::FUSE Threads::Threads)
# disable darwin extensions for now
target_compile_definitions(openvfsfuse PRIVATE FUSE_DARWIN_ENABLE_EXTENSIONS=0)

install(FILES config.json DESTINATION ${KDE_INSTALL_CONFDIR}/openvfs)
install(TARGETS openvfsfuse EXPORT OpenVFSConfig ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
