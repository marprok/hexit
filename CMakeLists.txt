cmake_minimum_required(VERSION 3.23)

project(hexit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME})
add_compile_options(-Wall -Wextra -Werror -O2)

find_package(Curses REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_sources(${PROJECT_NAME} PRIVATE
	src/hexit.cc
	src/terminal_window.cc
	src/data_buffer.cc
)

target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})
