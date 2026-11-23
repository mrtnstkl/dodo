cmake_minimum_required(VERSION 3.13)
project(dodo VERSION 0.1.0 LANGUAGES C CXX)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(dodo_examples_default ON)
else()
    set(dodo_examples_default OFF)
endif()
option(DODO_BUILD_EXAMPLES "Build dodo example programs" ${dodo_examples_default})

# Dependencies
find_package(OpenGL REQUIRED)

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

target_compile_features(dodo PUBLIC cxx_std_20)
set_target_properties(dodo PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(dodo PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:-Wno-conversion-null;-Wno-volatile;-Wno-pointer-arith>"
    )
endif()

# Public compile definitions for users of the library
target_compile_definitions(dodo PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

# Include directories for users of the library (PUBLIC)
target_include_directories(dodo
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb>
)

# GLFW: build it and link it into dodo
if(NOT TARGET glfw)
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_EXAMPLES OFF)
    add_subdirectory(vendor/glfw)
endif()

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
