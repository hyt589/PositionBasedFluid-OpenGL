cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(pbf VERSION 0.1.0)

include(CTest)
enable_testing()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_TESTS off)
set(ASSIMP_BUILD_TOOLS off)

set(JSON_BuildTests OFF CACHE INTERNAL "")


add_subdirectory(libs/glfw)
add_subdirectory(libs/json)
add_subdirectory(libs/assimp)

set(LIBS glfw nlohmann_json::nlohmann_json assimp)

include_directories(
    "src/" 
    "libs/glad/include" 
    "libs/imgui"
    "libs/glm/glm"
    "libs/json/single_include/nlohmann"
    "libs/assimp/include"
    "libs/stb/"
)

file(GLOB SOURCE_FILES 
    "src/glad.c" 
    "src/*.cpp" 
    "libs/imgui/imgui.cpp"
    "libs/imgui/examples/imgui_impl_opengl3.cpp"
    "libs/imgui/examples/imgui_impl_glfw.cpp"
    "libs/imgui/imgui_draw.cpp"
    "libs/imgui/imgui_widgets.cpp"
)

add_executable(pbf ${SOURCE_FILES})

target_link_libraries(pbf ${LIBS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
