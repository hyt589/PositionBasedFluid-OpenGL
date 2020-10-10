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
    float dt, lastx, lasty;
    int mode = 0;
    bool focus = false, mouseInit = false;
public:
    Renderer(JSON & config);
    ~Renderer(){
        delete pbrProgram;
        delete shadowProgram;
        delete cam;
    };
    void render(Scene & scene);

    void processInput(GLFWwindow * window);

    friend void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);

    friend void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods);

    friend void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

    friend void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};

