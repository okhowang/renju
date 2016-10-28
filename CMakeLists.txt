cmake_minimum_required(VERSION 3.5)
project(renju)

include_directories(${CMAKE_SOURCE_DIR}/rapidjson/include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    main.cpp
    Renju.cpp)

add_executable(renju ${SOURCE_FILES})

install(TARGETS renju
        CONFIGURATIONS Release
        DESTINATION C:/xampp/cgi-bin)

add_executable(make_table make_table.cpp)