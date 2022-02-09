cmake_minimum_required(VERSION 3.21)
project(assignment1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lGL -lGLU -lglut")
set(CMAKE_CXX_STANDARD 14)

add_executable(assignment1 main.cpp InitShader.cpp)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARIES})