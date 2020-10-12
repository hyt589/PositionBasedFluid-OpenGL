#include "app.hpp"
#include "gui.hpp"

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
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, (int)config["renderer"]["glVersion"]["major"]);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (int)config["renderer"]["glVersion"]["minor"]);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    viewWidth = config["renderer"]["resolution"]["width"];
    viewHeight = config["renderer"]["resolution"]["height"];
    fov = config["renderer"]["fov"];

    auto monitor = glfwGetPrimaryMonitor();
    auto vmode = glfwGetVideoMode(monitor);
    if (viewHeight > vmode->height || viewWidth > vmode->width)
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
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOG_ERR("Application:: Failed to initialize GLAD")
        BREAK_POINT;
        exit(1);
        return;
    }
    LOG_INFO("Application:: window created")

    LOG_INFO("Application:: Confiuration complete")
}

void R::OpenGLApplication::init()
{
    //load scene;
    auto model = std::make_unique<Model>(config["assets"]["scene"]);
    model->scale = 0.01f;
    scene.addModel(glm::vec3(0.f), glm::vec3(0.f), std::move(model));
    scene.addLight(glm::vec3(-1.0f, 18.0f, 1.0f), glm::vec3(1.f), 1500.f);

    LOG_INFO("Application:: Scene loaded")

    //init camera
    cam = new Camera(
        glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

    //init imgui
    guiInit();

    //load shaders
    auto pbrVertShader = std::make_unique<Shader>(ShaderType::VERT, (std::string)config["assets"]["shaders"]["vshader"]);
    auto pbrFragShader = std::make_unique<Shader>(ShaderType::FRAG, (std::string)config["assets"]["shaders"]["fshader"]);
    std::vector<std::unique_ptr<Shader>> pbrShaders;
    pbrShaders.push_back(std::move(pbrVertShader));
    pbrShaders.push_back(std::move(pbrFragShader));
    shaders["lighting"] = new Program(pbrShaders);

    auto depthVertShader = std::make_unique<Shader>(ShaderType::VERT, (std::string)config["assets"]["depthShaders"]["v"]);
    auto depthFragShader = std::make_unique<Shader>(ShaderType::FRAG, (std::string)config["assets"]["depthShaders"]["f"]);
    auto depthGeomShader = std::make_unique<Shader>(ShaderType::GEO, (std::string)config["assets"]["depthShaders"]["g"]);
    std::vector<std::unique_ptr<Shader>> depthShaders;
    depthShaders.push_back(std::move(depthVertShader));
    depthShaders.push_back(std::move(depthGeomShader));
    depthShaders.push_back(std::move(depthFragShader));
    shaders["shadow"] = new Program(depthShaders);

    LOG_INFO("Application:: Shaders loaded");

    //configuring renderer
    renderer.appWindow = appWindow;
    renderer.shaderProgram = shaders["lighting"];

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
        "OpenGL Renderer", std::function([&app, img]() -> void {
            GuiGroupPanel(
                "Scene", std::function([&app, img]() -> void {
                    ImGui::Image((ImTextureID)img, ImVec2(app.viewWidth, app.viewHeight), ImVec2(0, 1), ImVec2(1, 0));
                }));

            GuiGroupPanel(
                "Menu", std::function([&app, img]() -> void {
                    if (ImGui::Button("Close App"))
                    {
                        glfwSetWindowShouldClose(app.appWindow, true);
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

    while (!glfwWindowShouldClose(appWindow))
    {
        GL(glClearColor(0.f, 0.f, 0.f, 0.f));
        GL(glClear(GL_COLOR_BUFFER_BIT));
        //processInput
        GLtex2D targetTex(GL_RGBA32F, GL_RGBA, GL_FLOAT, viewWidth, viewHeight);
        // renderer.renderPassTex(scene, *cam, renderTarget);
        // guiRender(guiFunc, guiParams);
        // guiRender(std::function(AppGui), *this, renderTarget);
        AppGui(*this, targetTex.ID());

        glfwSwapBuffers(appWindow);
        glfwPollEvents();
    }

    LOG_INFO("Application:: Terminating");
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
