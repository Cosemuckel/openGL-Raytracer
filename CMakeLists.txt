cmake_minimum_required(VERSION 3.5)

# Set project name and version
project(renderer VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Set compiler flags to -O2
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
 "src/Source.cpp"
 "src/Bodies.h"
 "src/Shader.h"
 "src/Initialization.h"
 "src/Structures.h"
 "src/TraceSimple.h")

# Add GLFW library
add_subdirectory("dependencies/glfw-3.3.8")
include_directories("dependencies/glfw-3.3.8/include")

# Add GLEW library
add_subdirectory("dependencies/glew-2.2.0/build/cmake")
include_directories("dependencies/glew-2.2.0/include")

# Add glm headers
include_directories("dependencies/g-truc-glm-bf71a83")

# Add stb headers
include_directories("dependencies/stb")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} glew_s)

# Copy the executable to the root directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:${PROJECT_NAME}>
		${CMAKE_SOURCE_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>)