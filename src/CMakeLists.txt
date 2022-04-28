set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# Source file storage
add_library(project_source INTERFACE)

add_subdirectory(splayer)

# Standalone executable
add_executable(${CMAKE_PROJECT_NAME} main.cpp)

# Link our project libraries
add_library(project_libraries INTERFACE)

target_include_directories(project_source INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(project_source INTERFACE
    -DDEF_PROJECT_NAME="${CMAKE_PROJECT_NAME}"
)

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
    target_link_libraries(project_libraries
        INTERFACE
        "-framework OpenGL"
    )
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
