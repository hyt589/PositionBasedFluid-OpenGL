#pragma once
#include "global.hpp"
#include "ShaderProgram.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "GLabstractions.hpp"

enum ShaderMode
{
    SHADOW_MAP,
    LIGHTING,
    LIGHT_SOURCE
};

class Renderer
{
public:
    virtual void renderPass() = 0;
    virtual void renderPassTex(const GLtex2D &tex) = 0;
};

class Ogl_PbrShadowmap_Renderer : Renderer
{
private:
public:
    Program *shaderProgram;
    GLframebuffer *fb;
    GLframebuffer *dfb[MAX_LIGHTS];
    GLtexCubeMap *depthCubemap[MAX_LIGHTS];
    GLrenderbuffer *rb;
    Scene *scene;
    Camera *cam;
    int viewWidth, viewHeight, shadowWidth, shadowHeight;
    float fov, znear, zfar;
    unsigned int sphereVAO = 0;
    unsigned int indexCount;

    std::unordered_map<ShaderMode, Program *> shaders;

    Ogl_PbrShadowmap_Renderer(){};
    ~Ogl_PbrShadowmap_Renderer()
    {
        // delete ggxLightingProgram;
        // delete shadowCubemapProgram;
        delete shaderProgram;
        delete fb;
        delete rb;
        // delete scene;
        delete cam;
    };
    void init()
    {
        fb = new GLframebuffer;
        rb = new GLrenderbuffer;
        for (size_t i = 0; i < MAX_LIGHTS; i++)
        {
            dfb[i] = new GLframebuffer;
            depthCubemap[i] = new GLtexCubeMap(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, shadowWidth, shadowHeight);
        }

        configureShadowmap();
    }
    void renderPass() override;
    void renderPassTex(const GLtex2D &tex) override;
    void configurShader(ShaderMode mode, int light_pass = 0);
    void configureBuffers(const GLtex2D &tex);
    void configureShadowmap();
    void renderLightSource(Light &ls);
};