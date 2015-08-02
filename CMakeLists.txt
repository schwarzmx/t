cmake_minimum_required(VERSION 2.6)
project(t)

set(VERSION "0.0.1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /usr/local/lib)

include_directories(include)

add_subdirectory(src)

add_custom_target(run
    COMMAND t
    DEPENDS t
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)