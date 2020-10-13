#include "renderer.hpp"
#include "windowUtil.hpp"
#include "gui.hpp"

void R::Ogl_PbrShadowmap_Renderer::configurShadowmap()
{
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        dfb[i]->bindAndDo(std::function([this, i]() -> void {
            GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap[i]->ID(), 0));
            GL(glDrawBuffer(GL_NONE));
            GL(glReadBuffer(GL_NONE));
            auto fbStatus = GL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
            if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
            {
                LOG_ERR("Framebuffer incomplete!");
                BREAK_POINT;
            }
        }));
    }
}

void R::Ogl_PbrShadowmap_Renderer::renderPass()
{
    //render shadowmaps
    GL(glViewport(0, 0, shadowWidth, shadowHeight));
    for (size_t i = 0; i < scene->numLights; i++)
    {
        dfb[i]->bindAndDo(std::function([this, i]() -> void {
            GL(glEnable(GL_DEPTH_TEST));
            GL(glClearColor(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX));
            GL(glClear(GL_DEPTH_BUFFER_BIT));

            configurShader(ShaderMode::SHADOW_MAP, i);
            scene->render(*shaderProgram, false);

            GL(glDisable(GL_DEPTH_TEST));
        }));
    }

    //render final result to a texture that shows up in an ImGui window
    fb->bindAndDo(std::function([this]() -> void {
        GL(glViewport(0, 0, viewWidth, viewHeight));
        GL(glEnable(GL_DEPTH_TEST));
        GL(glClearColor(0.7f, 0.7f, 0.7f, 1.0f));
        GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        configurShader(ShaderMode::LIGHTING);
        scene->render(*shaderProgram, true);
        configurShader(ShaderMode::LIGHT_SOURCE);
        for(int i = 0; i < scene->numLights; i++)
        {
            renderLightSource(scene->lights[i]);
        }
        GL(glDisable(GL_DEPTH_TEST));
    }));
}

void R::Ogl_PbrShadowmap_Renderer::configurBuffers(const GLtex2D &target)
{
    // target.bind();
    fb->bindAndDo(std::function([this, &target]() -> void {
        attachTexToFramebuffer(target, GL_COLOR_ATTACHMENT0);
        rb->bind();
        GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewWidth, viewHeight));
        GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb->ID()));
        auto fbStatus = GL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
        if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_ERR("Framebuffer incomplete!");
            BREAK_POINT;
        }
        rb->unbind();
    }));
    // target.unbind();
}

void R::Ogl_PbrShadowmap_Renderer::renderPassTex(const GLtex2D &target)
{
    fb->bindAndDo(std::function([this]() -> void {
        renderPass();
    }));
}

void R::Ogl_PbrShadowmap_Renderer::configurShader(ShaderMode mode, int i)
{
    if (!shaders.count(mode))
    {
        LOG_ERR("Renderer:: No shader for mode " << mode);
        BREAK_POINT;
        exit(1);
    }
    shaderProgram = shaders[mode];
    switch (mode)
    {
    case ShaderMode::LIGHTING:
    {
        float aspect = (float)viewWidth / (float)viewHeight;
        auto mat_projection = glm::perspective(glm::radians(fov), aspect, znear, zfar);
        auto mat_view = cam->getViewMatrix();
        shaderProgram->setUniform("mat_projection", mat_projection, glUniformMatrix4fv);
        shaderProgram->setUniform("mat_view", mat_view, glUniformMatrix4fv);
        shaderProgram->setUniform("far_plane", zfar, glUniform1f);
        shaderProgram->setUniform("numLights", scene->numLights, glUniform1i);
        for (int i = 0; i < scene->numLights; i++)
        {
            shaderProgram->setUniform("lightPos[" + std::to_string(i) + "]", scene->lights[i].position, glUniform3fv);
            shaderProgram->setUniform("lightColor[" + std::to_string(i) + "]", scene->lights[i].color * scene->lights[i].emission, glUniform3fv);
        }
        shaderProgram->setUniform("camPos", cam->pos, glUniform3fv);
        // shaderProgram->setUniform("mode", 3, glUniform1i);

        for (int j = 0; j < scene->numLights; j++)
        {
            GL(glActiveTexture(GL_TEXTURE0 + j));
            shaderProgram->setUniform("depthCubeMap[" + std::to_string(j) + "]", j, glUniform1i);
            depthCubemap[j]->bind();
        }
        break;
    }
    case ShaderMode::SHADOW_MAP:
    {
        float aspect = 1.0;
        auto shadowProj = glm::perspective(glm::radians(fov), 1.f, znear, zfar);
        shaderProgram->setUniform("far_plane", zfar, glUniform1f);
        auto lightPos = scene->lights[i].position;
        shaderProgram->setUniform("lightPos", lightPos, glUniform3fv);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        for (int j = 0; j < 6; j++)
        {
            shaderProgram->setUniform("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[j], glUniformMatrix4fv);
        }
        break;
    }
    case ShaderMode::LIGHT_SOURCE:
    {
        float aspect = (float)viewWidth / (float)viewHeight;
        auto mat_projection = glm::perspective(glm::radians(fov), aspect, znear, zfar);
        auto mat_view = cam->getViewMatrix();
        shaderProgram->setUniform("mat_projection", mat_projection, glUniformMatrix4fv);
        shaderProgram->setUniform("mat_view", mat_view, glUniformMatrix4fv);
        break;
    }
    default:
        break;
    }
}

void R::Ogl_PbrShadowmap_Renderer::renderLightSource(Light & ls)
{
    auto mat_model = glm::mat4(0);
    mat_model = glm::translate(mat_model, ls.position);
    mat_model = glm::scale(mat_model, glm::vec3(10.f));
    shaderProgram->setUniform("mat_model", mat_model, glUniformMatrix4fv);
    shaderProgram->setUniform("color", ls.color, glUniform3fv);
    shaderProgram->activate();
    if (sphereVAO == 0)
    {
        GL(glGenVertexArrays(1, &sphereVAO));

        unsigned int vbo, ebo;
        GL(glGenBuffers(1, &vbo));
        GL(glGenBuffers(1, &ebo));

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359;
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = indices.size();

        std::vector<float> data;
        for (std::size_t i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
        }
        GL(glBindVertexArray(sphereVAO));
        GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
        GL(glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW));
        GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
        GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW));
        float stride = (3 + 2 + 3) * sizeof(float);
        GL(glEnableVertexAttribArray(0));
        GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0));
        GL(glEnableVertexAttribArray(1));
        GL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))));
        GL(glEnableVertexAttribArray(2));
        GL(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float))));
    }

    GL(glBindVertexArray(sphereVAO));
    GL(glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0));
    shaderProgram->deactivate();
}

//=========================================================================
//=============================below is deprecate==========================
//=========================================================================

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
    guiWindow = glfwCreateWindow(400, 600, "GUI", NULL, window);

    glfwGetWindowSize(window, &w, &h);
    if (window == NULL)
    {
        LOG_ERR("Renderer:: Failed to create glfw window")
        glfwTerminate();
        exit(1);
        return;
    }
    if (guiWindow == NULL)
    {
        LOG_ERR("Renderer:: GUI window creation failed")
        glfwTerminate();
        exit(1);
        return;
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetWindowUserPointer(guiWindow, this);

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
    auto pbrGeomShader = std::make_unique<Shader>(ShaderType::GEO, (std::string)config["assets"]["shaders"]["gshader"]);
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
        glm::vec3(-1.f, 18.0f, 1.f),
        glm::vec3(0.0, -1.0, 0.0),
        glm::vec3(0.0, 1.0, 0.0));

    float aspect_ratio = (float)config["renderer"]["resolution"]["width"] / (float)config["renderer"]["resolution"]["height"];
#define Z_NEAR 0.01f
#define Z_FAR 50.f

    auto mat_perspective_projection = glm::perspective(
        glm::radians((float)config["renderer"]["fov"]),
        aspect_ratio,
        Z_NEAR, Z_FAR);

    std::string name = "mat_projection";
    pbrProgram->setUniform(name, mat_perspective_projection, glUniformMatrix4fv);
    pbrProgram->setUniform("far_plane", Z_FAR, glUniform1f);

    float shadow_aspect = (float)shadow_width / (float)shadow_height;
    shadowProj = glm::perspective(glm::radians(90.f), shadow_aspect, Z_NEAR, Z_FAR);

    LOG_INFO("Renderer::Initialization complete.")
}

void Renderer::render(Scene &scene)
{

    // #ifndef NDEBUG
    // GL(glEnable(GL_DEBUG_OUTPUT));
    // GL(glDebugMessageCallback( MessageCallback, 0 ));
    // #endif

    uint depthCubeMap[MAX_LIGHTS], depthMapFBO[MAX_LIGHTS];
    // GL(glGenFramebuffers(MAX_LIGHTS, depthMapFBO));
    // GL(glGenTextures(MAX_LIGHTS, depthCubeMap));
    for (int i = 0; i < scene.numLights; i++)
    {
        GL(glActiveTexture(GL_TEXTURE0 + i));
        GL(glGenFramebuffers(1, &(depthMapFBO[i])));
        GL(glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]));
        GL(glGenTextures(1, &(depthCubeMap[i])));
        GL(glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap[i]));

        GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
        GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0));
        GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0));

        for (uint j = 0; j < 6; j++)
        {
            GL(glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + j,
                0, GL_DEPTH_COMPONENT, shadow_width, shadow_height,
                0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
        }

        GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap[i], 0));
        GL(glDrawBuffer(GL_NONE));
        GL(glReadBuffer(GL_NONE));
        auto fbStatus = GL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
        if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_ERR("Framebuffer incomplete!");
            BREAK_POINT;
        }
        else
        {
            LOG_INFO("Depth buffer " + std::to_string(depthMapFBO[i]) + " complete")
            LOG_INFO("depthCUbeMap[" + std::to_string(i) + "] = " + std::to_string(depthCubeMap[i]))
        }
        GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    GL(glEnable(GL_DEPTH_TEST));
    GL(glEnable(GL_CULL_FACE));

    //init imgui
    // glfwMakeContextCurrent(guiWindow);
    IMGUI_CHECKVERSION();
    auto guiContext = ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(guiWindow, true);
    // ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
    glfwMakeContextCurrent(window);

    LOG_INFO("Rendering starting");
    float lastFrame = 0;
    while (!glfwWindowShouldClose(window) && !glfwWindowShouldClose(guiWindow))
    {
        // glfwMakeContextCurrent(guiWindow);
        // GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        glfwMakeContextCurrent(window);

        float now = glfwGetTime();
        dt = now - lastFrame;
        lastFrame = now;
        float fps = 1.f / dt;

        glfwSetWindowTitle(window, std::string("mode: " + std::to_string(mode % 5)).c_str());

        processInput(window);

        pbrProgram->setUniform("numLights", scene.numLights, glUniform1i);

        GL(glClearColor(0.1f, 0.1f, 0.1f, 1.f));
        GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        GL(glEnable(GL_DEPTH_TEST));

        //render shadow map
        for (int i = 0; i < scene.numLights; i++)
        {

            auto &light = scene.lights[i];
            light.position = cam->pos;

            std::vector<glm::mat4> shadowTransforms;
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

            GL(glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]));

            GL(glViewport(0, 0, shadow_width, shadow_height));
            GL(glClearColor(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX));

            GL(glClear(GL_DEPTH_BUFFER_BIT));
            shadowProgram->setUniform("far_plane", Z_FAR, glUniform1f);
            for (int j = 0; j < shadowTransforms.size(); j++)
            {
                shadowProgram->setUniform(
                    "shadowMatrices[" + std::to_string(j) + "]",
                    shadowTransforms[j],
                    glUniformMatrix4fv);
            }
            shadowProgram->setUniform("lightPos", light.position, glUniform3fv);

            scene.render(*shadowProgram, false);

            // GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
            // GL(glDrawBuffers(1, drawBuffers));

            GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        }

        //render scene with shadow map

        // glfwGetWindowSize(window, &w, &h);

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

        if (mode % 5 == 0)
        {
            glfwSetWindowSize(window, shadow_width, shadow_height);
            GL(glViewport(0, 0, shadow_width * 2, shadow_height * 2)); //times 2 on macOS
            scene.render(*shadowProgram, false);
        }
        else
        {
            glfwSetWindowSize(window, w, h);
            GL(glViewport(0, 0, w * 2, h * 2)); //times 2 on macOS
            scene.render(*pbrProgram, true);
        }

        //imgui
        glfwMakeContextCurrent(guiWindow);
        GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("Menu");
            {
                ImGui::BeginGroupPanel("Info");
                ImGui::Text("Frame Rate: %f", fps);
                for (int i = 0; i < scene.numLights; i++)
                {
                    auto &pos = scene.lights[i].position;
                    auto &color = scene.lights[i].color;
                    ImGui::Text("Light pos[%d]: (%f, %f, %f)", i, pos.x, pos.y, pos.z);
                    ImGui::Text("Light color[%d] : (%f, %f, %f)", i, color.x, color.y, color.z);
                }
                ImGui::Text("Cam Pos: (%f, %f, %f)", cam->pos.x, cam->pos.y, cam->pos.z);
                ImGui::EndGroupPanel();
            }
            {
                ImGui::BeginGroupPanel("Light parameters");
                for (int i = 0; i < scene.numLights; i++)
                {
                    auto &light = scene.lights[i];
                    float color[3] = {light.color.x, light.color.y, light.color.z};
                    ImGui::ColorPicker3("Color", color);
                    light.color = glm::vec3(color[0], color[1], color[2]);
                    float pos[3] = {light.position.x, light.position.y, light.position.z};
                    ImGui::NewLine();
                    ImGui::SliderFloat3("Position", pos, -80.f, 80.f, "%f", 1.0f);
                    light.position = glm::vec3(pos[0], pos[1], pos[2]);
                    ImGui::SliderFloat("Emission", &light.emission, 0.f, 3000.f, "%f", 1.f);
                }
                ImGui::EndGroupPanel();
            }
            // {
            //     ImGui::BeginGroupPanel("Image");
            //     ImGui::Image((ImTextureID)71, ImVec2(400, 400), ImVec2(0, 1), ImVec2(1, 0));
            //     // ImGui::ShowMetricsWindow();
            //     ImGui::EndGroupPanel();
            // }

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(guiWindow);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwDestroyWindow(guiWindow);
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

#undef Z_NEAR
#undef Z_FAR