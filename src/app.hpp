#pragma once

#include "global.hpp"
#include "camera.hpp"
#include "asset.hpp"
#include "ShaderProgram.hpp"

class Application
{
private:
    JSON config;
    GLFWwindow * window;
    Camera * cam;
    Model * model;
    Program * program;

public:
    Application(std::string path);
    void init();
    void renderLoop();
    void run();
    ~Application();
};


