# Add subdirectories with dependencies
add_subdirectory(glad)
add_subdirectory(stb)
include(${CMAKE_CURRENT_LIST_DIR}/glfw.cmake)

# FC is set to be disconnected in order to stop downloading
set(FETCHCONTENT_FULLY_DISCONNECTED ON CACHE BOOL "set to 'off' to begin downloading again" FORCE)
