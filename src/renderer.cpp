#include "renderer.hpp"
#include "windowUtil.hpp"

#ifndef NDEBUG
void GLAPIENTRY
MessageCallback(GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar *message,
                const void *userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
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

    glfwSetWindowUserPointer(window, this);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, keyCallBack);
    glfwSetMouseButtonCallback(window, mouseButtonCallBack);
    glfwSetCursorPosCallback(window, cursorPosCallback);

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
    depthShaders.push_back(std::move(depthGeomShader));
    depthShaders.push_back(std::move(depthFragShader));
    pbrProgram = new Program(pbrShaders);
    shadowProgram = new Program(depthShaders);

    shadow_width = config["renderer"]["shadowMap"]["width"];
    shadow_height = config["renderer"]["shadowMap"]["height"];

    cam = new Camera(
        glm::vec3(0.f, 8.0f, 1.f),
        glm::vec3(1.0, 0.0, 0.0),
        glm::vec3(0.0, 1.0, 0.0));

    // cam = new Camera(
    //     glm::vec3(0.f, 1200.0f, 0.f),
    //     glm::vec3(0.0, -1.0, 0.0),
    //     glm::vec3(0.0, 0.0, -1.0));

    float aspect_ratio = (float)config["renderer"]["resolution"]["width"] / (float)config["renderer"]["resolution"]["height"];
    float near = 0.01f, far = 300.f;

    auto mat_perspective_projection = glm::perspective(
        glm::radians((float)config["renderer"]["fov"]),
        aspect_ratio,
        near, far);

    std::string name = "mat_projection";
    pbrProgram->setUniform(name, mat_perspective_projection, glUniformMatrix4fv);
    pbrProgram->setUniform("far_plane", far, glUniform1f);

    shadowProgram->setUniform("far_plane", far, glUniform1f);

    float shadow_aspect = (float)shadow_width / (float)shadow_height;
    shadowProj = glm::perspective(glm::radians(90.f), shadow_aspect, near, far);

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

    
    // #ifndef NDEBUG
    // GL(glEnable(GL_DEBUG_OUTPUT));
    // GL(glDebugMessageCallback( MessageCallback, 0 ));
    // #endif

    uint depthCubeMap[MAX_LIGHTS], depthMapFBO[MAX_LIGHTS];
    for (int i = 0; i < scene.numLights; i++)
    {
        GL(glGenFramebuffers(1, depthMapFBO + i));
        GL(glGenTextures(1, depthCubeMap + i));
        GL(glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap[i]));
        for (uint j = 0; j < 6; j++)
        {
            GL(glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + j,
                0, GL_DEPTH_COMPONENT, shadow_width, shadow_height,
                0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
            GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
        }
        GL(glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]));
        GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap[i], 0));
        GL(glDrawBuffer(GL_NONE));
        GL(glReadBuffer(GL_NONE));
        GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    //init imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    GL(glEnable(GL_DEPTH_TEST));
    GL(glEnable(GL_CULL_FACE));

    float lastFrame = 0;
    while (!glfwWindowShouldClose(window))
    {

        float now = glfwGetTime();
        dt = now - lastFrame;
        lastFrame = now;
        float fps = 1.f / dt;

        glfwSetWindowTitle(window, std::string("mode: " + std::to_string(mode%3) + ", FPS: " + std::to_string(fps)).c_str());

        processInput(window);

        GL(glClearColor(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX));

        pbrProgram->setUniform("numLights", scene.numLights, glUniform1i);

        //render shadow map
        GL(glViewport(0, 0, shadow_width, shadow_height));
        for (int i = 0; i < scene.numLights; i++)
        {
            GL(glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]));
            GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            auto light = scene.lights[i];

            std::vector<glm::mat4> shadowTransforms;
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));


            for (int j = 0; j < shadowTransforms.size(); j++)
            {
                shadowProgram->setUniform(
                    "shadowMatrices[" + std::to_string(j) + "]",
                    shadowTransforms[j],
                    glUniformMatrix4fv);
            }
            shadowProgram->setUniform("lightPos", light.position, glUniform3fv);

            scene.render(*shadowProgram);

            GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
            
        }

        //render scene with shadow map
        // GL(glClearColor(0.1f, 0.2f, 0.3f, 1.f));
        GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        glfwGetWindowSize(window, &w, &h);
        GL(glViewport(0, 0, w * 2, h * 2)); //times 2 on macOS
        auto mat_view = cam->getViewMatrix();
        pbrProgram->setUniform("mode", mode, glUniform1i);
        pbrProgram->setUniform("mat_view", mat_view, glUniformMatrix4fv);
        pbrProgram->setUniform("camPos", cam->pos, glUniform3fv);
        for (int i = 0; i < scene.numLights; i++)
        {
            pbrProgram->setUniform("lightPos[" + std::to_string(i) + "]", scene.lights[i].position, glUniform3fv);
            pbrProgram->setUniform("lightColor[" + std::to_string(i) + "]", scene.lights[i].color * scene.lights[i].emission, glUniform3fv);
            pbrProgram->activate();
            GL(glActiveTexture(GL_TEXTURE0 + i));
            pbrProgram->setUniform("depthCubeMap[" + std::to_string(i) + "]", i, glUniform1i);
            GL(glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap[i]));
        }
        scene.render(*pbrProgram);

        //imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        bool showWindow = true;
        // ImGui::ShowDemoWindow(&showWindow);
        ImGui::Begin("Shit", &showWindow);
        ImGui::Text("fuck me");
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Renderer::processInput(GLFWwindow *window)
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


