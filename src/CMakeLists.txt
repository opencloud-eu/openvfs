include(ECMMarkNonGuiExecutable)

add_subdirectory(openvfs)
add_subdirectory(openvfsfuse)
add_subdirectory(vfsstat)

install(EXPORT OpenVFSConfig DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/openvfs NAMESPACE OpenVFS::)
