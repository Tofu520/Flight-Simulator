cmake_minimum_required(VERSION 3.14)
project(FlightSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Freetype REQUIRED)

file(GLOB SOURCES "src/*.cpp" "src/*.c")
add_executable(FlightSimulator ${SOURCES})

target_include_directories(FlightSimulator PRIVATE include)

target_link_libraries(FlightSimulator
    PRIVATE
        OpenGL::GL
        glfw
        glm::glm
        Freetype::Freetype
)
