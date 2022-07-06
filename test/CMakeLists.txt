enable_testing()

add_executable(data_buffer_test)

target_include_directories(data_buffer_test PRIVATE ../src)

target_sources(data_buffer_test PRIVATE
	data_buffer_test.cc
	../src/data_buffer.cc
        ../src/data_cache.cc
)

target_link_libraries(data_buffer_test gtest_main)

include(GoogleTest)
gtest_discover_tests(data_buffer_test)
