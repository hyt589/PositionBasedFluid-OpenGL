#include "app.hpp"
#include "gui.hpp"

namespace R
{
    void cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
    {

        auto app = static_cast<R::OpenGLApplication *>(glfwGetWindowUserPointer(window));
        if (app->focused)
        {
            if (!app->mouseInit)
            {
                app->lastCursorX = xpos;
                app->lastCursorY = ypos;
                app->mouseInit = true;
            }

            float xoffset = xpos - app->lastCursorX;
            float yoffset = app->lastCursorY - ypos;
            app->lastCursorX = xpos;
            app->lastCursorY = ypos;

            xoffset *= app->renderer.cam->sensitivity;
            yoffset *= app->renderer.cam->sensitivity;

            app->renderer.cam->yaw += xoffset;
            app->renderer.cam->pitch += yoffset;

            if (app->renderer.cam->pitch > 89.f)
            {
                app->renderer.cam->pitch = 89.f;
            }
            if (app->renderer.cam->pitch < -89.f)
            {
                app->renderer.cam->pitch = -89.f;
            }
            glm::vec3 nextDir(
                cos(glm::radians(app->renderer.cam->yaw)) * cos(glm::radians(app->renderer.cam->pitch)),
                sin(glm::radians(app->renderer.cam->pitch)),
                sin(glm::radians(app->renderer.cam->yaw)) * cos(glm::radians(app->renderer.cam->pitch)));

            app->renderer.cam->dir = nextDir;
        }
    }
} // namespace R
void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            unsigned int id,
                            GLenum severity,
                            GLsizei length,
                            const char *message,
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        std::cout << "Source: API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cout << "Source: Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cout << "Source: Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cout << "Source: Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cout << "Source: Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cout << "Source: Other";
        break;
    }
    std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "Type: Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "Type: Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "Type: Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "Type: Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "Type: Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cout << "Type: Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cout << "Type: Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cout << "Type: Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "Type: Other";
        break;
    }
    std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "Severity: high";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "Severity: medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "Severity: low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "Severity: notification";
        break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

Application::Application(JSON config) : renderer(Renderer(config))
{
    auto model = std::make_unique<Model>(config["assets"]["scene"]);
    model->scale = 0.01f;
    scene.addModel(glm::vec3(0.f), glm::vec3(0.f), std::move(model));
    scene.addLight(glm::vec3(-1.0f, 18.0f, 1.0f), glm::vec3(1.f), 1500.f);
}

void Application::run()
{
    renderer.render(scene);
}

//==========================================================================

R::OpenGLApplication::OpenGLApplication(JSON &j) : config(j)
{
    int gl_v_major = config["renderer"]["glVersion"]["major"];
    int gl_v_minor = config["renderer"]["glVersion"]["minor"];
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, (int)config["renderer"]["glVersion"]["major"]);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (int)config["renderer"]["glVersion"]["minor"]);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#endif // !NDEBUG

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    renderer.viewWidth = config["renderer"]["resolution"]["width"];
    renderer.viewHeight = config["renderer"]["resolution"]["height"];
    renderer.fov = config["renderer"]["fov"];
    renderer.zfar = config["renderer"]["zfar"];
    renderer.znear = config["renderer"]["znear"];

    auto monitor = glfwGetPrimaryMonitor();
    auto vmode = glfwGetVideoMode(monitor);
    if (renderer.viewWidth > vmode->width || renderer.viewHeight > vmode->height)
    {
        LOG_WARN("Viewport size is larger than monitor size")
        LOG_WARN("Rendered scene might not fit on screen")
        BREAK_POINT;
    }

    std::string title = config["renderer"]["windowTitle"];
    appWindow = glfwCreateWindow(vmode->width, vmode->height, title.c_str(), NULL, NULL);

    if (appWindow == NULL)
    {
        LOG_ERR("Application:: Failed to create glfw window")
        glfwTerminate();
        BREAK_POINT;
        exit(1);
        return;
    }
    glfwMakeContextCurrent(appWindow);
    // glfwSetWindowAttrib(appWindow, GLFW_VISIBLE, GLFW_FALSE);
    glfwSetWindowAttrib(appWindow, GLFW_DECORATED, GLFW_FALSE);
    glfwSetWindowAttrib(appWindow, GLFW_MAXIMIZED, GLFW_TRUE);
    glfwSetWindowUserPointer(appWindow, this);
    glfwSetCursorPosCallback(appWindow, cursorPosCallback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOG_ERR("Application:: Failed to initialize GLAD")
        BREAK_POINT;
        exit(1);
        return;
    }
    LOG_INFO("Application:: window created")

#ifndef NDEBUG
    if (gl_v_major * 10 + gl_v_minor >= 43)
    {
        GL(glEnable(GL_DEBUG_OUTPUT));
        GL(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
        GL(glDebugMessageCallback(glDebugOutput, nullptr));
        GL(glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE));
    }
#endif // !NDEBUG

    LOG_INFO("Application:: Confiuration complete")
}

void R::OpenGLApplication::init()
{
    //load scene;
    auto model = std::make_unique<Model>(config["assets"]["scene"]);
    model->scale = 0.01f;
    scene.addModel(glm::vec3(0.f), glm::vec3(0.f), std::move(model));
    scene.addLight(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(1.f), 150.f);
    renderer.scene = &scene;

    LOG_INFO("Application:: Scene loaded")

    //init camera
    renderer.cam = new Camera(
        glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

    //init imgui
    guiInit();

    //load shaders
    auto pbrVertShader = std::make_unique<Shader>(ShaderType::VERT, (std::string)config["assets"]["shaders"]["vshader"]);
    auto pbrFragShader = std::make_unique<Shader>(ShaderType::FRAG, (std::string)config["assets"]["shaders"]["fshader"]);
    std::vector<std::unique_ptr<Shader>> pbrShaders;
    pbrShaders.push_back(std::move(pbrVertShader));
    pbrShaders.push_back(std::move(pbrFragShader));
    renderer.shaders[ShaderMode::LIGHTING] = new Program(pbrShaders);

    auto depthVertShader = std::make_unique<Shader>(ShaderType::VERT, (std::string)config["assets"]["depthShaders"]["v"]);
    auto depthFragShader = std::make_unique<Shader>(ShaderType::FRAG, (std::string)config["assets"]["depthShaders"]["f"]);
    auto depthGeomShader = std::make_unique<Shader>(ShaderType::GEO, (std::string)config["assets"]["depthShaders"]["g"]);
    std::vector<std::unique_ptr<Shader>> depthShaders;
    depthShaders.push_back(std::move(depthVertShader));
    depthShaders.push_back(std::move(depthGeomShader));
    depthShaders.push_back(std::move(depthFragShader));
    renderer.shaders[ShaderMode::SHADOW_MAP] = new Program(depthShaders);

    LOG_INFO("Application:: Shaders loaded");

    renderer.shadowWidth = config["renderer"]["shadowMap"]["width"];
    renderer.shadowHeight = config["renderer"]["shadowMap"]["height"];
    renderer.init();
    LOG_INFO("Application:: Renderer configured");

    isInit = true;
    LOG_INFO("Application:: Initialization Complete");
}

void AppGui(R::OpenGLApplication &app, GLuint img)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    int w, h;
    glfwGetWindowSize(app.appWindow, &w, &h);
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    GuiWindow(
        "OpenGL Renderer",
        std::function([&app, img, w, h]() -> void {
            GuiGroupPanel(
                "Menu",
                std::function([&app, img, w, h]() -> void {
                    if (ImGui::Button("Close App"))
                    {
                        glfwSetWindowShouldClose(app.appWindow, true);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Control Camera"))
                    {
                        app.focused = true;
                        glfwSetInputMode(app.appWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Shadow on"))
                    {
                        app.renderer.shaders[ShaderMode::LIGHTING]->setUniform("mode", 2, glUniform1i);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Shadow off"))
                    {
                        app.renderer.shaders[ShaderMode::LIGHTING]->setUniform("mode", 1, glUniform1i);
                    }
                    GuiGroupPanel(
                        "Scene",
                        std::function([&app, img, w, h]() -> void {
                            ImGui::Image((ImTextureID)img, ImVec2(3 * w / 4, 3 * h / 4), ImVec2(0, 1), ImVec2(1, 0));
                            glCheckError;
                        }),
                        ImVec2(0, h));
                }));

            GuiGroupPanel(
                "View matrix",
                std::function([&app]() -> void {
                    auto m = app.renderer.cam->getViewMatrix();
                    for (int i = 0; i < 4; i++)
                    {
                        ImGui::Text("%f, %f, %f, %f", m[0][i], m[1][i], m[2][i], m[3][i]);
                    }
                }));
        }));

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void R::OpenGLApplication::run()
{
    if (!isInit)
    {
        LOG_ERR("Application:: Not initialized");
        LOG_ERR("Application:: Quitting");
        BREAK_POINT;
        return;
    }

    LOG_INFO("Application:: Running");

    // renderer.fb->bind();
    GLtex2D targetTex(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, renderer.viewWidth, renderer.viewHeight);
    // renderer.fb->unbind();

    renderer.configurBuffers(targetTex);

    // GL(glEnable(GL_DEPTH_TEST));

    renderer.shaders[ShaderMode::LIGHTING]->setUniform("mode", 1, glUniform1i);

    float lastFrame = 0;
    while (!glfwWindowShouldClose(appWindow))
    {
        GL(glClearColor(0.f, 0.f, 0.f, 0.f));
        GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        float now = glfwGetTime();
        dt = now - lastFrame;
        lastFrame = now;
        float fps = 1.f / dt;

        processInput(appWindow);
        renderer.renderPass();
        AppGui(*this, targetTex.ID());
        // AppGui(*this, renderer.depthCubemap[0]->ID());

        glfwSwapBuffers(appWindow);
        glfwPollEvents();
    }

    LOG_INFO("Application:: Terminating");
    glfwTerminate();
}

void R::OpenGLApplication::guiInit()
{
    IMGUI_CHECKVERSION();
    auto guiContext = ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    std::string fontPath = config["fonts"]["firaMedium"];
    int fontSize = config["fontSize"];
    io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize);
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(appWindow, true);
    // ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
    LOG_INFO("Application:: GUI initialized");
}

void R::OpenGLApplication::processInput(GLFWwindow *window)
{
    Camera *cam = renderer.cam;
    if (focused)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            auto forward = glm::vec3(cam->dir.x, 0.f, cam->dir.z);
            forward = glm::normalize(forward);
            cam->pos += forward * cam->speed * dt;
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            auto forward = glm::vec3(cam->dir.x, 0.f, cam->dir.z);
            auto left = glm::cross(glm::vec3(0, 1, 0), forward);
            left = glm::vec3(left.x, 0.f, left.z);
            left = glm::normalize(left);
            cam->pos += left * cam->speed * dt;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            auto forward = glm::vec3(cam->dir.x, 0.f, cam->dir.z);
            auto left = glm::cross(glm::vec3(0, 1, 0), forward);
            left = glm::vec3(left.x, 0.f, left.z);
            left = glm::normalize(left);
            cam->pos -= left * cam->speed * dt;
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            auto forward = glm::vec3(cam->dir.x, 0.f, cam->dir.z);
            forward = glm::normalize(forward);
            cam->pos -= forward * cam->speed * dt;
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            cam->pos += glm::vec3(0, 1, 0) * cam->speed * dt;
        }

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            cam->pos -= glm::vec3(0, 1, 0) * cam->speed * dt;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        focused = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}
