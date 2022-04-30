# MIT License
#
# Copyright (c) 2022 Bennett Anderson
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

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
    -DLOGGER_HEADER="${CMAKE_PROJECT_NAME}"
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
