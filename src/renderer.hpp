#pragma once
#include "global.hpp"
#include "ShaderProgram.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "GLabstractions.hpp"

class Renderer
{
private:
    Program * pbrProgram;
    Program * shadowProgram;
    GLFWwindow * window;
    GLFWwindow * guiWindow;
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



namespace R
{
    class Renderer
    {
    public:
        virtual void renderPass(const Scene & s, const Camera & cam) = 0;
        virtual void renderPassTex(const Scene & s, const Camera & cam, const I_GLtex & tex) = 0;
    };

    class Ogl_PbrShadowmap_Renderer : Renderer
    {
    private:
    public:
        GLFWwindow * appWindow;
        Program * shaderProgram;
        glm::mat4 * mat_projection;
        Ogl_PbrShadowmap_Renderer(){};
        ~Ogl_PbrShadowmap_Renderer(){
            // delete ggxLightingProgram;
            // delete shadowCubemapProgram;
            delete shaderProgram;
        };
        void renderPass(const Scene & s, const Camera & cam) override;
        void renderPassTex(const Scene & s, const Camera & cam, const I_GLtex & tex) override;
    };
}