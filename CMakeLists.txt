cmake_minimum_required(VERSION 3.5)
project(minecraft_sv VERSION 0.1.0 LANGUAGES C CXX)



include(CTest)
enable_testing()
find_package(uuid_v4)
target_link_libraries(uuid_v4::uuid_v4)
add_compile_options(-mavx2)

add_executable(minecraft_sv main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CXX_STANDARD 20)
include(CPack)

