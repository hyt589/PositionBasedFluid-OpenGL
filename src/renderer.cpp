#include "renderer.hpp"
#include "windowUtil.hpp"

#ifndef NDEBUG
void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}
#endif // !NDEBUG

Renderer::Renderer(JSON &config)
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

    window = glfwCreateWindow(config["renderer"]["resolution"]["width"],
                              config["renderer"]["resolution"]["height"],
                              title.c_str(),
                              NULL, NULL);

    glfwGetWindowSize(window, &w, &h);
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

    auto pbrVertShader = std::make_unique<Shader>(ShaderType::VERT, (std::string)config["assets"]["shaders"]["vshader"]);
    auto pbrFragShader = std::make_unique<Shader>(ShaderType::FRAG, (std::string)config["assets"]["shaders"]["fshader"]);
    auto depthVertShader = std::make_unique<Shader>(ShaderType::VERT, (std::string)config["assets"]["depthShaders"]["v"]);
    auto depthFragShader = std::make_unique<Shader>(ShaderType::FRAG, (std::string)config["assets"]["depthShaders"]["f"]);
    auto depthGeomShader = std::make_unique<Shader>(ShaderType::GEO, (std::string)config["assets"]["depthShaders"]["g"]);
    std::vector<std::unique_ptr<Shader>> pbrShaders;
    std::vector<std::unique_ptr<Shader>> depthShaders;
    pbrShaders.push_back(std::move(pbrVertShader));
    pbrShaders.push_back(std::move(pbrFragShader));
    depthShaders.push_back(std::move(depthVertShader));
    depthShaders.push_back(std::move(depthFragShader));
    depthShaders.push_back(std::move(depthGeomShader));
    pbrProgram = new Program(pbrShaders);
    shadowProgram = new Program(depthShaders);

    shadow_width = config["renderer"]["shadowMap"]["width"];
    shadow_height = config["renderer"]["shadowMap"]["height"];

    cam = new Camera(
        glm::vec3(0.f, 200.0f, 0.f),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f));

    float aspect_ratio = (float)config["renderer"]["resolution"]["width"] / (float)config["renderer"]["resolution"]["height"];

    auto mat_perspective_projection = glm::perspective(
        glm::radians((float)config["renderer"]["fov"]),
        aspect_ratio,
        0.01f, 100000.f);

    std::string name = "mat_projection";
    pbrProgram->setUniform(name, mat_perspective_projection, glUniformMatrix4fv);
    pbrProgram->setUniform("far_plane", 100000.f, glUniform1f);

    float shadow_aspect = (float)shadow_width / (float)shadow_height;
    shadowProj = glm::perspective(glm::radians(90.f), shadow_aspect, 0.01f, 100000.f);

    LOG_INFO("Renderer::Initialization complete.")
}

void Renderer::render(Scene &scene)
{
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
    GL(glEnable(GL_CULL_FACE));
    // #ifndef NDEBUG
    // GL(glEnable(GL_DEBUG_OUTPUT));
    // GL(glDebugMessageCallback( MessageCallback, 0 ));
    // #endif
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        GL(glClearColor(0.1f, 0.2f, 0.3f, 1.0f));

        pbrProgram->setUniform("numLights", scene.numLights, glUniform1i);

        //render shadow map
        GL(glViewport(0, 0, shadow_width, shadow_height));
        for (int i = 0; i < scene.numLights; i++)
        {
            auto light = scene.lights[i];

            //creating FBO and cubemap texture
            GL(glClear(GL_DEPTH_BUFFER_BIT));
            uint depthCubeMap, depthMapFBO;
            GL(glGenTextures(1, &depthCubeMap));
            GL(glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap));
            GL(glGenFramebuffers(1, &depthMapFBO));
            for (uint j = 0; j < 6; j++)
            {
                GL(glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + j,
                    0, GL_DEPTH_COMPONENT, shadow_width, shadow_height,
                    0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
                GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
                GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
                GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
                GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
                GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
            }

            std::vector<glm::mat4> shadowTransforms;
            shadowTransforms.push_back(shadowProj *
                                       glm::lookAt(light.position, light.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(shadowProj *
                                       glm::lookAt(light.position, light.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(shadowProj *
                                       glm::lookAt(light.position, light.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
            shadowTransforms.push_back(shadowProj *
                                       glm::lookAt(light.position, light.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
            shadowTransforms.push_back(shadowProj *
                                       glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(shadowProj *
                                       glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

            // attach depth texture as FBO's depth buffer
            GL(glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO));
            GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0));
            GL(glDrawBuffer(GL_NONE));
            GL(glReadBuffer(GL_NONE));

            for (int j = 0; j < shadowTransforms.size(); j++)
            {
                shadowProgram->setUniform(
                    "shadowMatrices[" + std::to_string(j) + "]",
                    shadowTransforms[i],
                    glUniformMatrix4fv);
            }
            shadowProgram->setUniform("far_plane", 100000.f, glUniform1f);
            shadowProgram->setUniform("lightPos", light.position, glUniform3fv);

            scene.render(*shadowProgram);

            GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

            pbrProgram->with([](Program * p, std::unordered_map<std::string, std::any> params){
                int i = std::any_cast<int>(params["i"]);
                GL(glActiveTexture(GL_TEXTURE0 + i));
                p->setUniform(
                    "depthCubeMap["+ std::to_string(i) + "]",
                    i, glUniform1i
                );
                GL(glBindTexture(GL_TEXTURE_CUBE_MAP, std::any_cast<uint>(params["depthCubeMap"])));
                GL(glActiveTexture(GL_TEXTURE0));
            }, std::unordered_map<std::string, std::any>({
                {"i", i},
                {"depthCubeMap", depthCubeMap}
            }));
        }

        //render scene with shadow map
        GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        glfwGetWindowSize(window, &w, &h);
        GL(glViewport(0, 0, w*2, h*2));//times 2 for msaa
        auto mat_view = cam->getViewMatrix();
        pbrProgram->setUniform("mat_view", mat_view, glUniformMatrix4fv);
        pbrProgram->setUniform("camPos", cam->pos, glUniform3fv);
        for (int i = 0; i < scene.numLights; i++)
        {
            pbrProgram->setUniform("lightPos[" + std::to_string(i) + "]", scene.lights[i].position, glUniform3fv);
            pbrProgram->setUniform("lightColor[" + std::to_string(i) + "]", scene.lights[i].color * scene.lights[i].emission, glUniform3fv);
        }
        scene.render(*pbrProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}