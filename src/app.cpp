#include "app.hpp"

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

Application::Application(std::string path)
{
    std::ifstream in(path);
    in >> config;

    init();
}

void Application::init()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config["renderer"]["glVersion"]["major"]);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config["renderer"]["glVersion"]["minor"]);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    
    LOG_INFO("Application::Initialization complete.")
}

void Application::run()
{

    std::string title = config["renderer"]["windowTitle"];

    window = glfwCreateWindow(config["renderer"]["resolution"]["width"],
                              config["renderer"]["resolution"]["height"],
                              title.c_str(),
                              NULL, NULL);
    if (window == NULL)
    {
        LOG_ERR("Application:: Failed to create glfw window")
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOG_ERR("Application:: Failed to initialize GLAD")
        return;
    }

    model = new Model(config["assets"]["scene"]);
    auto vshader = std::make_unique<Shader>(ShaderType::VERT, (std::string) config["assets"]["shaders"]["vshader"]);
    auto fshader = std::make_unique<Shader>(ShaderType::FRAG, (std::string) config["assets"]["shaders"]["fshader"]);
    std::vector<std::unique_ptr<Shader>> shaders;
    shaders.push_back(std::move(vshader));
    shaders.push_back(std::move(fshader));
    program = new Program(shaders);

    while (!glfwWindowShouldClose(window))
    {
        renderLoop();
    }

    glfwTerminate();
}

void Application::renderLoop()
{
    // input
    // -----
    processInput(window);

    // render
    // ------
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
}

Application::~Application()
{
}