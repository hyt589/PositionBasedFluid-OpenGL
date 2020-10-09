#pragma once
#include "global.hpp"
#include "ShaderProgram.hpp"
#include "scene.hpp"
#include "camera.hpp"

class Renderer
{
private:
    Program * pbrProgram;
    Program * shadowProgram;
    GLFWwindow * window;
    Camera * cam;
    uint shadow_width, shadow_height;
    glm::mat4 shadowProj;
    int w, h;
public:
    Renderer(JSON & config);
    ~Renderer(){
        delete pbrProgram;
        delete shadowProgram;
        delete cam;
    };
    void render(Scene & scene);
};

