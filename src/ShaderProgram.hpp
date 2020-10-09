#pragma once

#include "global.hpp"

enum ShaderType
{
    VERT, FRAG, GEO, TESS_CTL, TESS_EVAL, COMP
};

class Shader
{
private:
    /* data */
public:
    unsigned int ID;
    ShaderType type;
    bool compilation_success = false;
    Shader(ShaderType type, std::string path);
    ~Shader();
};

class Program
{
private:
    /* data */
public:
    unsigned int ID;
    bool linking_success = false;
    Program(std::vector<std::unique_ptr<Shader>> & shaders);
    ~Program();

    void activate();
    void deactivate();

    template<typename T>
    void setUniform(std::string name, T value, void (*f)(GLint,T)){
        activate();
        auto loc = GL(glGetUniformLocation(ID, name.c_str()));
        if(loc == -1){
            LOG_ERR("Uniform not found: " + name)
            return;
        }
        GL((*f)(loc, value));
        deactivate();
        // LOG_INFO("Uniform '" + name + "' set")
    };

    template<typename T, typename P>
    void setUniform(std::string name, T value, void (*f)(GLint, GLsizei, P)){
        activate();
        auto loc = GL(glGetUniformLocation(ID, name.c_str()));
        if(loc == -1){
            LOG_ERR("Uniform not found: " + name)
            return;
        }
        GL((*f)(loc, 1, glm::value_ptr(value)));
        deactivate();
        // LOG_INFO("Uniform '" + name + "' set")
    };

    template<typename T, typename P>
    void setUniform(std::string name, T value, void (*f)(GLint, GLsizei, GLboolean, P)){
        activate();
        auto loc = GL(glGetUniformLocation(ID, name.c_str()));
        if(loc == -1){
            LOG_ERR("Uniform not found: " + name)
            return;
        }
        GL((*f)(loc, 1, GL_FALSE, glm::value_ptr(value)));
        deactivate();
        // LOG_INFO("Uniform '" + name + "' set")
    };
};


