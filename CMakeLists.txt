cmake_minimum_required(VERSION 3.10)
project(BlockchainQuery)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Debug for better debugging support
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Add debug symbols
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

# Find CURL
find_package(CURL REQUIRED)

# Find Threading
find_package(Threads REQUIRED)

# Find OpenSSL for WebSocket WSS support
find_package(OpenSSL REQUIRED)

# Add executable
add_executable(blockchain_query main.cpp blockchain_client.cpp)

# Link CURL, Threads, and OpenSSL
target_link_libraries(blockchain_query PRIVATE CURL::libcurl Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

# Include websocketpp directory
target_include_directories(blockchain_query PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Installation
install(TARGETS blockchain_query DESTINATION bin)
