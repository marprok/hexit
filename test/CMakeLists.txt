enable_testing()

# Copy the file that is used in the unit test cases
file(COPY ${CMAKE_SOURCE_DIR}/hex_mode.png DESTINATION ${CMAKE_BINARY_DIR}/test)
file(RENAME ${CMAKE_BINARY_DIR}/test/hex_mode.png ${CMAKE_BINARY_DIR}/test/data.test)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "&{CMAKE_BINARY_DIR}")
add_executable(DataBufferTest)

target_include_directories(DataBufferTest PRIVATE ../src)

target_sources(DataBufferTest PRIVATE
	DataBufferTest.cc
	../src/DataBuffer.cc
    ../src/ChunkCache.cc
)

target_link_libraries(DataBufferTest gtest_main)

include(GoogleTest)
gtest_discover_tests(DataBufferTest)
