enable_testing()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "&{CMAKE_BINARY_DIR}")
add_executable(DataBufferTest)

target_include_directories(DataBufferTest PRIVATE ../src)

target_sources(DataBufferTest PRIVATE
	data_buffer_test.cc
        data_cache_test.cc
	../src/DataBuffer.cc
        ../src/ChunkCache.cc
)

target_link_libraries(DataBufferTest gtest_main)

include(GoogleTest)
gtest_discover_tests(DataBufferTest)
