add_executable(t t.cc jsoncpp.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

# install(FILES t DESTINATION _install)
