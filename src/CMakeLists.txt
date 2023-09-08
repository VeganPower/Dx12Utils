add_executable(test)

set(source main.cpp)


set_target_properties(test PROPERTIES BINARY_DIR "./bin")
set_target_properties(test PROPERTIES WIN32_EXECUTABLE ON)

target_sources(test PRIVATE ${source})


target_compile_features(test PUBLIC cxx_std_17)

target_link_libraries(test PUBLIC d3d12.lib dxgi.lib unofficial::d3d12-memory-allocator imgui::imgui) #glm tinyobjloader ktx dxcompiler dxil WinPixEventRuntime)
