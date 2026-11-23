# Dodo

To use Dodo from a CMake project, place a checkout at `external/dodo` and link
your target to `dodo::dodo`.

```cmake
add_subdirectory(external/dodo)
target_link_libraries(my_app PRIVATE dodo::dodo)
```

The target provides Dodo's headers, dependency include paths, and C++20
requirement. The checkout includes GLAD, GLFW, GLM, and the stb image headers.
Builds require CMake 3.13 or later, a C++20 compiler, and the platform's OpenGL
development libraries. On Linux, GLFW also requires the development packages
for the selected window-system backend.

Examples are enabled by default for a standalone Dodo build. Set
`DODO_BUILD_EXAMPLES` to `ON` before `add_subdirectory` to enable them in a parent
project.
