cmake_minimum_required(VERSION 3.23)

project(hexit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Taken from https://google.github.io/googletest/quickstart-cmake.html
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(${PROJECT_NAME})
add_compile_options(-Wall -Wextra -Werror -O2)

add_subdirectory(test)

find_package(Curses REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_sources(${PROJECT_NAME} PRIVATE
	src/hexit.cc
	src/terminal_window.cc
	src/data_buffer.cc
)

target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})
