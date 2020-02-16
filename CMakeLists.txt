cmake_minimum_required(VERSION 2.4)

project(ip2geo)

include_directories(${PROJECT_SOURCE_DIR})
add_executable(ip2geo ip2geo.cpp)
target_link_libraries(ip2geo GeoIP)
