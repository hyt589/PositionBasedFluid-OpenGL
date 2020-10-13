#pragma once

#include "global.hpp"
#include "renderer.hpp"
#include "scene.hpp"

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
        OpenGLApplication(JSON &j);

        void init() override;

        void run() override;

        void guiInit();

        void processInput(GLFWwindow * window);


    };
} // namespace R
