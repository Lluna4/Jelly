cmake_minimum_required(VERSION 3.5)
project(jelly VERSION 0.1.0 LANGUAGES C CXX)



include(CTest)
enable_testing()
find_package(uuid_v4)
target_link_libraries(uuid_v4::uuid_v4)
add_compile_options(-mavx2 -std=c++20)

add_executable(jelly src/main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

