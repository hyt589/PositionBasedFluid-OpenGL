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
    Program(std::vector<unsigned int> & shaders);
    ~Program();

    void use();

    template<typename T>
    void setUniform(std::string& name, T value, void (*f)(unsigned int,T));
};


