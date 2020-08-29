#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <json.hpp>
#include <fstream>
#include <set>
#include <map>
#include <memory>

using JSON = nlohmann::json;

#define LOG_INFO(msg) std::cout << "[Info] " << msg << std::endl;
#define LOG_ERR(msg) std::cerr << "[Error] " << msg << std::endl;
#define LOG_WARN(msg) std::cout << "[Warning] " << msg << std::endl;

// namespace global{
//     inline JSON config;
//     inline bool config_loaded = false;

//     inline void load_config(std::string path){
//         std::ifstream in(path);
//         in >> config;
//         config_loaded = true;
//     }
// } 

// inline auto & CONFIG = global::config;
// inline float aspect_ratio = (float) CONFIG["renderer"]["resolution"]["width"] / (float)CONFIG["renderer"]["resolution"]["height"];
// inline auto mat_perspective_projection = glm::perspective(
//     glm::radians((float) CONFIG["renderer"]["fov"]), 
//     aspect_ratio,
//     0.01f, 100000.f
// );

// inline auto mat_orthographic_projection = glm::ortho(
//     -0.5f * aspect_ratio,
//     0.5f * aspect_ratio,
//     -0.5f,
//     -0.5f,
//     0.01f, 100000.f
// );

