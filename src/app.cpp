#include "app.hpp"

// unsigned int sphereVAO = 0;
// unsigned int indexCount;
// void renderSphere()
// {
//     if (sphereVAO == 0)
//     {
//         glGenVertexArrays(1, &sphereVAO);

//         unsigned int vbo, ebo;
//         glGenBuffers(1, &vbo);
//         glGenBuffers(1, &ebo);

//         std::vector<glm::vec3> positions;
//         std::vector<glm::vec2> uv;
//         std::vector<glm::vec3> normals;
//         std::vector<unsigned int> indices;

//         const unsigned int X_SEGMENTS = 64;
//         const unsigned int Y_SEGMENTS = 64;
//         const float PI = 3.14159265359;
//         for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
//         {
//             for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
//             {
//                 float xSegment = (float)x / (float)X_SEGMENTS;
//                 float ySegment = (float)y / (float)Y_SEGMENTS;
//                 float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
//                 float yPos = std::cos(ySegment * PI);
//                 float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

//                 positions.push_back(glm::vec3(xPos, yPos, zPos));
//                 uv.push_back(glm::vec2(xSegment, ySegment));
//                 normals.push_back(glm::vec3(xPos, yPos, zPos));
//             }
//         }

//         bool oddRow = false;
//         for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
//         {
//             if (!oddRow) // even rows: y == 0, y == 2; and so on
//             {
//                 for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
//                 {
//                     indices.push_back(y       * (X_SEGMENTS + 1) + x);
//                     indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
//                 }
//             }
//             else
//             {
//                 for (int x = X_SEGMENTS; x >= 0; --x)
//                 {
//                     indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
//                     indices.push_back(y       * (X_SEGMENTS + 1) + x);
//                 }
//             }
//             oddRow = !oddRow;
//         }
//         indexCount = indices.size();

//         std::vector<float> data;
//         for (std::size_t i = 0; i < positions.size(); ++i)
//         {
//             data.push_back(positions[i].x);
//             data.push_back(positions[i].y);
//             data.push_back(positions[i].z);
//             if (uv.size() > 0)
//             {
//                 data.push_back(uv[i].x);
//                 data.push_back(uv[i].y);
//             }
//             if (normals.size() > 0)
//             {
//                 data.push_back(normals[i].x);
//                 data.push_back(normals[i].y);
//                 data.push_back(normals[i].z);
//             }
//         }
//         glBindVertexArray(sphereVAO);
//         glBindBuffer(GL_ARRAY_BUFFER, vbo);
//         glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
//         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
//         glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
//         float stride = (3 + 2 + 3) * sizeof(float);
//         glEnableVertexAttribArray(0);
//         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
//         glEnableVertexAttribArray(1);
//         glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
//         glEnableVertexAttribArray(2);
//         glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
//     }

//     glBindVertexArray(sphereVAO);
//     glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
// }

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

    cam = new Camera(
        glm::vec3(0.f),
        glm::vec3(0.f, 0.f, -1.f),
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
    
    renderLoop();

    glfwTerminate();
}

void Application::renderLoop()
{
    glm::vec3 lightPositions[] = {
        glm::vec3(0.0f, 0.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(150.0f, 150.0f, 150.0f),
    };

    while (!glfwWindowShouldClose(window))
    {
        glUseProgram(program->ID);
        // input
        // -----
        processInput(window);

        // render
        // ------
        auto mat_view = cam->getViewMatrix();
        program->setUniform("mat_view", mat_view, glUniformMatrix4fv);
        program->setUniform("camPos", cam->pos, glUniform3fv);
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i){
            glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
            program->setUniform("lightPos[" + std::to_string(i) + "]", lightPositions[i], glUniform3fv);
            program->setUniform("lightColor[" + std::to_string(i) + "]", lightColors[i], glUniform3fv);

        }
        model->Draw(*program);
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

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
    std::free(window);
}