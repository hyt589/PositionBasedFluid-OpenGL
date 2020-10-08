#pragma once

#include "global.hpp"
#include "renderer.hpp"
#include "scene.hpp"
class Application
{
private:
    // std::unique_ptr<Renderer> renderer;
    // std::unique_ptr<Scene> scene;
    Renderer renderer;
    Scene scene;
public:
    Application(JSON config);
    void run();
};


