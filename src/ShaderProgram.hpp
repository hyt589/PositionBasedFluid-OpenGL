#pragma once

#include "global.hpp"

enum ShaderType
{
    VERTEX, FRAGMENT
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
    Program(unsigned int vShaderID, unsigned int fShaderID);
    ~Program();

    void use();

    template<typename T>
    void setUniform(std::string& name, T value, void (*f)(unsigned int,T));
};


