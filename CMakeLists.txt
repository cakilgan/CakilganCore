cmake_minimum_required(VERSION 3.31.0)
project(CakilganCore VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)



file(GLOB_RECURSE SOURCES "src/*.cpp")
add_library(CakilganCore ${SOURCES})
target_include_directories(CakilganCore PUBLIC "include")


add_executable(CakilganCoreApp "main.cpp")
target_link_libraries(CakilganCoreApp PRIVATE CakilganCore)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
