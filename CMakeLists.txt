# Specifies the CMake version we require
cmake_minimum_required (VERSION 3.8)

# Set our Includes folder as the place to look for library includes
include_directories(${CMAKE_SOURCE_DIR}/Output/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Store the name of our project in a variable
set(EvoTest "EvoTest")
project (${EvoTest} LANGUAGES CXX)

# Specify the name of the project executable and which sources should be used in the project
add_executable(${EvoTest}
    Application.cpp
)

source_group(Source FILES Application.cpp)

# Specify the libraries to use when linking the executable
target_link_libraries (${EvoTest} ${CMAKE_SOURCE_DIR}/Output/lib/$<CONFIGURATION>/Autonomic.lib)
target_link_libraries (${EvoTest} ${CMAKE_SOURCE_DIR}/3rdParty/Libraries/glfw3.lib)
target_link_libraries (${EvoTest} ${CMAKE_SOURCE_DIR}/3rdParty/Libraries/$<CONFIGURATION>/GDT.lib)
target_link_libraries (${EvoTest} ${CMAKE_CURRENT_SOURCE_DIR}/box2d.lib)
