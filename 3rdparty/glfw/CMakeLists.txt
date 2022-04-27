# glfw version 3.3.4
set(DEP_GLFW_VERSION "3.3.6")
set(DEP_GLFW_LOC "https://github.com/glfw/glfw/archive/refs/tags/3.3.6.zip")

include(ExternalProject)

ExternalProject_Add(glfw-build
    URL                 ${DEP_GLFW_LOC}
    CMAKE_ARGS          -G "Unix Makefiles"
                        -DCMAKE_BUILD_TYPE=Release
                        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                        -DCMAKE_C_FLAGS=-fPIC
                        -DGLFW_BUILD_TESTS=FALSE
                        -DGLFW_BUILD_DOCS=False
                        -DBUILD_SHARED_LIBS=False
                        -DGLFW_BUILD_EXAMPLES=False
                        -DCMAKE_INSTALL_LIBDIR=lib
    BUILD_BYPRODUCTS    <INSTALL_DIR>/lib/libglfw3.a
)

ExternalProject_Get_Property(glfw-build INSTALL_DIR)

file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
add_library(glfw STATIC IMPORTED GLOBAL)
set_target_properties(glfw
    PROPERTIES
        IMPORTED_LOCATION ${INSTALL_DIR}/lib/libglfw3.a
        INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
        VERSION ${DEP_GLFW_VERSION}
)

if(APPLE)
    target_link_libraries(glfw INTERFACE
        "-framework IOKit"
        "-framework Cocoa"
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(glfw INTERFACE
        X11
    )
endif()

add_dependencies(glfw glfw-build)