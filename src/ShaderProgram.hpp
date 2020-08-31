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

    void use();

    template<typename T>
    void setUniform(std::string name, T value, void (*f)(GLint,T)){
        auto loc = glGetUniformLocation(ID, name.c_str());
        if(loc == -1){
            LOG_ERR("Uniform not found: " + name)
            return;
        }
        (*f)(loc, value);
        LOG_INFO("Uniform '" + name + "' set")
    };

    template<typename T, typename P>
    void setUniform(std::string name, T value, void (*f)(GLint, GLsizei, P)){
        auto loc = glGetUniformLocation(ID, name.c_str());
        if(loc == -1){
            LOG_ERR("Uniform not found: " + name)
            return;
        }
        (*f)(loc, 1, glm::value_ptr(value));
        LOG_INFO("Uniform '" + name + "' set")
    };

    template<typename T, typename P>
    void setUniform(std::string name, T value, void (*f)(GLint, GLsizei, GLboolean, P)){
        auto loc = glGetUniformLocation(ID, name.c_str());
        if(loc == -1){
            LOG_ERR("Uniform not found: " + name)
            return;
        }
        (*f)(loc, 1, GL_FALSE, glm::value_ptr(value));
        LOG_INFO("Uniform '" + name + "' set")
    };
};


