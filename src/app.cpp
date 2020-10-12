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
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    std::string title = config["renderer"]["windowTitle"];

    viewWidth = config["renderer"]["resolution"]["width"];
    viewHeight = config["renderer"]["resolution"]["height"];

    appWindow = glfwCreateWindow(1920,
                                 1080,
                                 title.c_str(),
                                 glfwGetPrimaryMonitor(), NULL);

    if (appWindow == NULL)
    {
        LOG_ERR("Application:: Failed to create glfw window")
        glfwTerminate();
        BREAK_POINT;
        exit(1);
        return;
    }
    glfwMakeContextCurrent(appWindow);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOG_ERR("Application:: Failed to initialize GLAD")
        BREAK_POINT;
        exit(1);
        return;
    }
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

    //init renderer
    renderer.appWindow = appWindow;
    auto pbrVertShader = std::make_unique<Shader>(ShaderType::VERT, (std::string)config["assets"]["shaders"]["vshader"]);
    auto pbrFragShader = std::make_unique<Shader>(ShaderType::FRAG, (std::string)config["assets"]["shaders"]["fshader"]);
    std::vector<std::unique_ptr<Shader>> pbrShaders;
    pbrShaders.push_back(std::move(pbrVertShader));
    pbrShaders.push_back(std::move(pbrFragShader));
    renderer.ggxLightingProgram = new Program(pbrShaders);

    LOG_INFO("Application:: Renderer configured");
}

void AppGui(R::OpenGLApplication &app, GLuint img)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    GuiWindow(
        "OpenGL Renderer", std::function([&app, img]() -> void {
            GuiGroupPanel(
                "Scene", std::function([&app, img]() -> void {
                    ImGui::Image((ImTextureID) 60, ImVec2(app.viewWidth, app.viewHeight), ImVec2(0,1), ImVec2(1,0));
                }));
        }));

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void R::OpenGLApplication::run()
{
    while (!glfwWindowShouldClose(appWindow))
    {
        GL(glClearColor(0.f, 0.f, 0.f, 1.f));
        GL(glClear(GL_COLOR_BUFFER_BIT));
        //processInput
        GLuint renderTarget;
        renderer.renderPassTex(scene, *cam, renderTarget);
        // guiRender(guiFunc, guiParams);
        // guiRender(std::function(AppGui), *this, renderTarget);
        AppGui(*this, renderTarget);

        glfwSwapBuffers(appWindow);
        glfwPollEvents();
    }
}

void R::OpenGLApplication::guiInit()
{
    IMGUI_CHECKVERSION();
    auto guiContext = ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(appWindow, true);
    // ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
    LOG_INFO("Application:: GUI initialized");
}


