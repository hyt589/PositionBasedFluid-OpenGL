#pragma once

#include <iostream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glm.hpp>
#include <json.hpp>
#include <fstream>

using JSON = nlohmann::json;

#define LOG_INFO(msg) std::cout << "[Info] " << msg << std::endl;
#define LOG_ERR(msg) std::cerr << "[Error] " << msg << std::endl;
#define LOG_WARN(msg) std::cout << "[Warning] " << msg << std::endl;

namespace global{
    JSON config;
    bool config_loaded = false;

    void load_config(std::string path){
        std::ifstream in(path);
        in >> config;
        config_loaded = true;
    }
} 

auto & CONFIG = global::config;