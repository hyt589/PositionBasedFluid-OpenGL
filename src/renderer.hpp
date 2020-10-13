#pragma once
#include "global.hpp"
#include "ShaderProgram.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "GLabstractions.hpp"
// #include "app.hpp"

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

enum ShaderMode{
    SHADOW_MAP, LIGHTING
};

namespace R
{
    class Renderer
    {
    public:
        virtual void renderPass() = 0;
        virtual void renderPassTex(const GLtex2D & tex) = 0;
    };

    class Ogl_PbrShadowmap_Renderer : Renderer
    {
    private:
    public:
        Program * shaderProgram;
        GLframebuffer * fb;
        GLframebuffer * dfb[MAX_LIGHTS];
        GLtexCubeMap * depthCubemap[MAX_LIGHTS];
        GLrenderbuffer * rb;
        Scene * scene;
        Camera * cam;
        int viewWidth, viewHeight, shadowWidth, shadowHeight;
        float fov, znear, zfar;
        std::unordered_map<ShaderMode, Program*> shaders;

        Ogl_PbrShadowmap_Renderer(){};
        ~Ogl_PbrShadowmap_Renderer(){
            // delete ggxLightingProgram;
            // delete shadowCubemapProgram;
            delete shaderProgram;
            delete fb;
            delete rb;
            // delete scene;
            delete cam;
        };
        void init(){
            fb = new GLframebuffer;
            rb = new GLrenderbuffer;
            for (size_t i = 0; i < MAX_LIGHTS; i++)
            {
                dfb[i] = new GLframebuffer;
                depthCubemap[i] = new GLtexCubeMap(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, shadowWidth, shadowHeight);
            }
            
            configurShadowmap();
        }
        void renderPass() override;
        void renderPassTex(const GLtex2D & tex) override;
        void configurShader(ShaderMode mode, int light_pass = 0);
        void configurBuffers(const GLtex2D & tex);
        void configurShadowmap();
    };
}