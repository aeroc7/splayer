set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# Source file storage
add_library(project_source INTERFACE)

# Standalone executable
add_executable(${CMAKE_PROJECT_NAME} main.cpp window.cpp)

# Link our project libraries
add_library(project_libraries INTERFACE)

if(WIN32)
    target_link_libraries(project_libraries
        INTERFACE
            -static
            -static-libgcc
            -static-libstdc++
            opengl32
    )
elseif(UNIX AND NOT APPLE)
   target_link_libraries(project_libraries
        INTERFACE
            -static-libgcc
            -static-libstdc++
            GL
   )
elseif(APPLE)
endif()

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        project_source
        project_options
        project_warnings
        project_libraries
        glfw
        glew
        ffmpeg
)
