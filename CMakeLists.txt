cmake_minimum_required(VERSION 3.5.0)
project(dodo VERSION 0.1.0 LANGUAGES C CXX)
cmake_policy(SET CMP0072 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(DODO_BUILD_EXAMPLES "Build dodo example programs" ON)

# Dependencies
find_package(OpenGL REQUIRED)

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-conversion-null -Wno-volatile -Wno-pointer-arith")
endif()

# ---- dodo library ----
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)
add_library(dodo STATIC
		${SOURCES}
    # vendor sources that must be compiled into the lib
    vendor/glad/src/glad.c
    vendor/stb_image.cpp
)

# Public compile definitions for users of the library
target_compile_definitions(dodo PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

# Include directories for users of the library (PUBLIC)
target_include_directories(dodo
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb>
)

set_property(TARGET dodo PROPERTY CXX_STANDARD 20)

# GLFW: build it and link it into dodo
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

target_link_libraries(dodo
    PUBLIC
        OpenGL::GL
        glfw
)

# Give a namespaced alias so other projects can use dodo::dodo
add_library(dodo::dodo ALIAS dodo)

# Optional: Visual Studio working directory (harmless elsewhere)
set_property(TARGET dodo PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# ---- Examples ----
if (DODO_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
