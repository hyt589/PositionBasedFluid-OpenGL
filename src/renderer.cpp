#include "renderer.hpp"
#include "windowUtil.hpp"

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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOG_ERR("Renderer:: Failed to initialize GLAD")
        exit(1);
        return;
    }

    auto pbrVertShader = std::make_unique<Shader>(ShaderType::VERT, (std::string) config["assets"]["shaders"]["vshader"]);
    auto pbrFragShader = std::make_unique<Shader>(ShaderType::FRAG, (std::string) config["assets"]["shaders"]["fshader"]);
    auto depthVertShader = std::make_unique<Shader>(ShaderType::VERT, (std::string) config["assets"]["depthShaders"]["v"]);
    auto depthFragShader = std::make_unique<Shader>(ShaderType::FRAG, (std::string) config["assets"]["depthShaders"]["f"]);
    std::vector<std::unique_ptr<Shader>> pbrShaders;
    std::vector<std::unique_ptr<Shader>> depthShaders;
    pbrShaders.push_back(std::move(pbrVertShader));
    pbrShaders.push_back(std::move(pbrFragShader));
    depthShaders.push_back(std::move(depthVertShader));
    depthShaders.push_back(std::move(depthFragShader));
    pbrProgram = new Program(pbrShaders);
    shadowProgram = new Program(depthShaders);

    shadow_width = config["renderer"]["shadowMap"]["width"];
    shadow_height = config["renderer"]["shadowMap"]["height"];

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
        processInput(window);
        GL(glClearColor(0.1f, 0.2f, 0.3f, 1.0f));
        GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        //render shadow map
        Camera shadowCam(glm::vec3(0.f), glm::vec3(1.f,0.f,0.f), glm::vec3(0.f,1.f,0.f));
        for(int i = 0; i < scene.numLights; i++){
            auto light = scene.lights[i];

            shadowCam.pos = light.position;

            //front
            shadowCam.dir = glm::vec3(0.f,0.f,-1.f);
            shadowCam.up = glm::vec3(0.f, 1.f, 0.f);
            auto light_view_mat = shadowCam.getViewMatrix();
            //back
            shadowCam.dir = glm::vec3(0.f, 0.f, 1.f);
            light_view_mat = shadowCam.getViewMatrix();
            //left
            shadowCam.dir = glm::vec3(-1.f, 0.f, 0.f);
            light_view_mat = shadowCam.getViewMatrix();
            //right
            shadowCam.dir = glm::vec3(1.f, 0.f, 0.f);
            light_view_mat = shadowCam.getViewMatrix();
            //up
            shadowCam.dir = glm::vec3(0.f, 1.f, 0.f);
            shadowCam.up = glm::vec3(0.f, 0.f, 1.f);
            light_view_mat = shadowCam.getViewMatrix();
            //down
            shadowCam.dir = glm::vec3(0.f, 1.f, 0.f);
            light_view_mat = shadowCam.getViewMatrix();

        }

        //render scene with shadow map
        auto mat_view = cam->getViewMatrix();
        pbrProgram->setUniform("mat_view", mat_view, glUniformMatrix4fv);
        pbrProgram->setUniform("camPos", cam->pos, glUniform3fv);
        for(int i = 0; i < scene.numLights; i++){
            pbrProgram->setUniform("lightPos[" + std::to_string(i) + "]", scene.lights[i].position, glUniform3fv);
            pbrProgram->setUniform("lightColor[" + std::to_string(i) + "]", scene.lights[i].color * scene.lights[i].emission, glUniform3fv);
        }
        scene.render(*pbrProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
}