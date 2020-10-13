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

namespace R
{
    class Application
    {
    public:
        virtual void init() = 0;
        virtual void run() = 0;
    };

    class OpenGLApplication : Application
    {
    private:
        bool isInit = false;
        float dt;
    public:
        JSON config;
        Scene scene;
        // Camera *cam;
        GLFWwindow *appWindow;
        Ogl_PbrShadowmap_Renderer renderer;
        bool focused = false, mouseInit = false;
        float lastCursorX, lastCursorY;
        // int viewWidth, viewHeight;
        // float fov, znear, zfar;
        // std::unordered_map<ShaderMode, Program*> shaders;
        //load config;
        OpenGLApplication(JSON &j);

        //load scene;
        //init cam;
        //create window and gl contex;
        //init ImGui
        //init renderer;
        void init() override;

        //while window should not close
        //  render scene to a texture
        void run() override;

        void guiInit();

        void processInput(GLFWwindow * window);


    };
} // namespace R
