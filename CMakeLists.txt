cmake_minimum_required(VERSION 3.21)
project(pong)

set(CMAKE_CXX_STANDARD 20)

find_library(GLUT glut)
find_package(OpenGL REQUIRED)
message("Glut is ${GLUT}")

add_executable(pong main.cpp)

target_compile_definitions(pong PUBLIC GL_SILENCE_DEPRECATION)

target_link_libraries(pong ${GLUT} ${OPENGL_LIBRARIES})