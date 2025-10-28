cmake_minimum_required(VERSION 3.15)

project(loadgen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB LOADGEN_SRC CONFIGURE_DEPENDS
  "src/*.cpp"
)

add_executable(loadgen ${LOADGEN_SRC})
target_link_libraries(loadgen PRIVATE CURL::libcurl pthread)
