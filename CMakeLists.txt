cmake_minimum_required(VERSION 3.5)
project(jelly VERSION 0.1.0 LANGUAGES C CXX)

# Include CTest for testing
include(CTest)
enable_testing()

# Disable building tests for the third-party library
set(JSON_BuildTests OFF CACHE INTERNAL "")

# Add the third-party library (nlohmann_json) as a subdirectory
add_subdirectory(nlohmann_json)
# Find and link the uuid_v4 library
#set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
# Set compile options
add_compile_options(-std=c++20)

# Define the executable
add_executable(jelly src/main.cpp)

# Link the third-party library to the executable
target_link_libraries(jelly PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(jelly PRIVATE OpenSSL::Crypto)

# Set CPack project name and version
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


