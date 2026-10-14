# examples/CMakeLists.txt

add_executable(1-hello 1-hello.cpp)
target_link_libraries(1-hello PRIVATE dodo::dodo)

add_executable(2-controlls 2-controlls.cpp)
target_link_libraries(2-controlls PRIVATE dodo::dodo)

add_executable(4-lighting 4-lighting.cpp)
target_link_libraries(4-lighting PRIVATE dodo::dodo)

add_executable(3-textured 3-textured.cpp)
target_link_libraries(3-textured PRIVATE dodo::dodo)

add_executable(5-uniforms 5-uniforms.cpp)
target_link_libraries(5-uniforms PRIVATE dodo::dodo)

add_executable(6-framebuffers 6-framebuffers.cpp)
target_link_libraries(6-framebuffers PRIVATE dodo::dodo)

add_executable(7-instancing 7-instancing.cpp)
target_link_libraries(7-instancing PRIVATE dodo::dodo)
