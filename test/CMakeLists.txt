enable_testing()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "&{CMAKE_BINARY_DIR}")
add_executable(HexitTest)

target_include_directories(HexitTest PRIVATE ../src)

target_sources(HexitTest PRIVATE
    DataBufferTest.cc
    SignatureReaderTest.cc
    ScrollerTest.cc
    ../src/DataBuffer.cc
    ../src/ChunkCache.cc
    ../src/FileHandler.cc
    ../src/SignatureReader.cc
    ../src/Scroller.cc
)

target_link_libraries(HexitTest gtest_main)

include(GoogleTest)
gtest_discover_tests(HexitTest)
