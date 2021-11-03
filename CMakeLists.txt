# The Project and minimum version requirement
cmake_minimum_required(VERSION 3.8)
project(PacMan_Game)


# Add subdirectory with dependencies
add_subdirectory(external)

# executable file creation
add_executable(${PROJECT_NAME})

# Additional source files added here
target_sources(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp)

# Include directories
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_SOURCE_DIR}/stb/include)

# STB compile definition
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  STB_IMAGE_IMPLEMENTATION)

# Linking to the external libraries
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    glad::glad
    glfw
    stbimage::stbimage)

# Copying the level0.txt file to current binary directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/level0.txt
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)

# Copying the pacman.png file to current binary directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/pacman.png
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)

