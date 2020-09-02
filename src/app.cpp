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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, (int) config["renderer"]["glVersion"]["major"]);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (int) config["renderer"]["glVersion"]["minor"]);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);


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

    cam = new Camera(
        glm::vec3(0.f, 100.0f, 0.f),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f)
    );

    float aspect_ratio = (float) config["renderer"]["resolution"]["width"] 
                        / (float)config["renderer"]["resolution"]["height"];

    auto mat_perspective_projection = glm::perspective(
        glm::radians((float) config["renderer"]["fov"]), 
        aspect_ratio,
        0.01f, 100000.f
    );

    std::string name = "mat_projection";
    program->setUniform(name, mat_perspective_projection, glUniformMatrix4fv);
    
    LOG_INFO("Application: Rendering starting...")
    renderLoop();
    LOG_INFO("Application: Rendering stopped")
    LOG_INFO("Application: Terminating...")

    glfwTerminate();
}

void Application::renderLoop()
{
    glm::vec3 lightPositions[] = {
        glm::vec3(10.0f, 1200.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(1500000.0f, 1500000.0f, 1500000.0f),
    };

    GL(glEnable(GL_DEPTH_TEST));
    GL(glEnable(GL_MULTISAMPLE));

    while (!glfwWindowShouldClose(window))
    {
        
        // input
        // -----
        processInput(window);

        // render
        // ------
        
        GL(glClearColor(0.1f, 0.2f, 0.3f, 1.0f));
        GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        auto mat_view = cam->getViewMatrix();
        program->setUniform("mat_view", mat_view, glUniformMatrix4fv);
        program->setUniform("camPos", cam->pos, glUniform3fv);
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i){
            glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
            program->setUniform("lightPos[" + std::to_string(i) + "]", lightPositions[i], glUniform3fv);
            program->setUniform("lightColor[" + std::to_string(i) + "]", lightColors[i], glUniform3fv);
        }
        model->Draw(*program);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

Application::~Application()
{
    delete program;
    delete model;
    delete cam;
}