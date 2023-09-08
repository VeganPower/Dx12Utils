cmake_minimum_required(VERSION 3.18.0)
project(Dx12_utils)

find_package(d3d12-memory-allocator CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
add_subdirectory(src)

#target_compile_features(core_utils PUBLIC cxx_std_17)


#target_link_libraries(core_utils PUBLIC d3d12.lib dxgi.lib glm tinyobjloader ktx dxcompiler dxil WinPixEventRuntime)
