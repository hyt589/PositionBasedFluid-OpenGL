#include "renderer.hpp"
#include "windowUtil.hpp"
#include "gui.hpp"

void Ogl_PbrShadowmap_Renderer::configurShadowmap()
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

void Ogl_PbrShadowmap_Renderer::renderPass()
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

void Ogl_PbrShadowmap_Renderer::configurBuffers(const GLtex2D &target)
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

void Ogl_PbrShadowmap_Renderer::renderPassTex(const GLtex2D &target)
{
    fb->bindAndDo(std::function([this]() -> void {
        renderPass();
    }));
}

void Ogl_PbrShadowmap_Renderer::configurShader(ShaderMode mode, int i)
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

void Ogl_PbrShadowmap_Renderer::renderLightSource(Light & ls)
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
