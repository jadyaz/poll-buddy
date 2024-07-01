cmake_minimum_required(VERSION 3.0)
project(VoteApp)

# Enable C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Find Boost package
find_package(Boost REQUIRED COMPONENTS system thread)

# Use pkg-config to find JsonCpp
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

# Include directories for Boost and JsonCpp
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Crow/include)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${JSONCPP_INCLUDE_DIRS})

# Add main.cpp as source file for the executable
add_executable(VoteApp src/main.cpp)

add_definitions(-DCROW_USE_BOOST)

# Link the Boost, JsonCpp libraries
target_link_libraries(VoteApp ${Boost_LIBRARIES} ${JSONCPP_LIBRARIES})