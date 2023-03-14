set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "&{CMAKE_BINARY_DIR}")
add_executable(HexitTest)

target_include_directories(HexitTest PRIVATE ../src)

target_sources(HexitTest PRIVATE
    ChunkCacheTest.cc
    ByteBufferTest.cc
    SignatureReaderTest.cc
    ScrollerTest.cc
    UtilitiesTest.cc
    IOHandlerMock.cc
    ../src/ByteBuffer.cc
    ../src/ChunkCache.cc
    ../src/FileHandler.cc
    ../src/SignatureReader.cc
    ../src/Scroller.cc
    ../src/Utilities.cc
)

target_link_libraries(HexitTest gtest_main)
add_test(NAME HexitTest COMMAND HexitTest )

include(GoogleTest)
gtest_discover_tests(HexitTest)
