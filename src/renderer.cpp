#include "renderer.hpp"

void framebuffer_size_callbackk(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void processInputt(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

Renderer::Renderer(JSON & config)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, (int) config["renderer"]["glVersion"]["major"]);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (int) config["renderer"]["glVersion"]["minor"]);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);


#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    std::string title = config["renderer"]["windowTitle"];

    window = glfwCreateWindow(config["renderer"]["resolution"]["width"],
                              config["renderer"]["resolution"]["height"],
                              title.c_str(),
                              NULL, NULL);

    if (window == NULL)
    {
        LOG_ERR("Renderer:: Failed to create glfw window")
        glfwTerminate();
        exit(1);
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callbackk);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOG_ERR("Renderer:: Failed to initialize GLAD")
        exit(1);
        return;
    }

    auto pbrVertShader = std::make_unique<Shader>(ShaderType::VERT, (std::string) config["assets"]["shaders"]["vshader"]);
    auto pbrFragShader = std::make_unique<Shader>(ShaderType::FRAG, (std::string) config["assets"]["shaders"]["fshader"]);
    std::vector<std::unique_ptr<Shader>> pbrShaders;
    pbrShaders.push_back(std::move(pbrVertShader));
    pbrShaders.push_back(std::move(pbrFragShader));
    pbrProgram = new Program(pbrShaders);
    shadowProgram = new Program(pbrShaders);

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
    pbrProgram->setUniform(name, mat_perspective_projection, glUniformMatrix4fv);
    LOG_INFO("Renderer::Initialization complete.")
    
}

void Renderer::render(Scene & scene){
    if (window == NULL)
    {
        LOG_ERR("Renderer:: Failed to create glfw window")
        glfwTerminate();
        return;
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOG_ERR("Renderer:: Failed to initialize GLAD");
        return;
    }

    GL(glEnable(GL_DEPTH_TEST));
    while (!glfwWindowShouldClose(window))
    {
        processInputt(window);
        GL(glClearColor(0.1f, 0.2f, 0.3f, 1.0f));
        GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        //render shadow map

        //render scene
        auto mat_view = cam->getViewMatrix();
        pbrProgram->setUniform("mat_view", mat_view, glUniformMatrix4fv);
        pbrProgram->setUniform("camPos", cam->pos, glUniform3fv);
        scene.render(*pbrProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
}