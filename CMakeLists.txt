cmake_minimum_required(VERSION 3.5)
project(renju)

include_directories(${CMAKE_SOURCE_DIR}/rapidjson/include ${CMAKE_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        main.cpp
        Renju.cpp
		Renju.hpp
        ${CMAKE_BINARY_DIR}/table.cpp)

add_executable(renju ${SOURCE_FILES})

install(TARGETS renju
        CONFIGURATIONS Release
        DESTINATION ${CMAKE_INSTALL_PREFIX})

add_executable(make_table make_table.cpp Renju.hpp)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/table.cpp
        COMMAND make_table
        DEPENDS make_table)