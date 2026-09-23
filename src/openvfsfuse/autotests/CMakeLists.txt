# The socket and job handling live in the openvfsfuse executable, so the test
# compiles those sources directly rather than linking a library.
add_executable(socketthreadtest
        socketthreadtest.cpp
        ../socketthread.cpp
        ../sharedmap.cpp
        ../strtools.cpp
)
target_include_directories(socketthreadtest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(socketthreadtest PRIVATE nlohmann_json::nlohmann_json Threads::Threads)
ecm_mark_as_test(socketthreadtest)

add_test(NAME socketthreadtest COMMAND socketthreadtest)
