add_executable(strtools_test
        strtools_test.cpp
        ../strtools.cpp
)
add_test(NAME strtools_test COMMAND strtools_test)
