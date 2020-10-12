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
    public:
        JSON config;
        Scene scene;
        Camera *cam;
        GLFWwindow *appWindow;
        Ogl_PbrShadowmap_Renderer renderer;
        int viewWidth, viewHeight;
        float fov;
        std::unordered_map<std::string, Program*> shaders;
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

        // template <typename... Args>
        // void guiRender(std::function<void(Args...)> guiFunc, Args & ... params)
        // {
        //     ImGui_ImplOpenGL3_NewFrame();
        //     ImGui_ImplGlfw_NewFrame();
        //     ImGui::NewFrame();
        //     guiFunc(params...);
        //     ImGui::Render();
        //     ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // };

    };
} // namespace R
