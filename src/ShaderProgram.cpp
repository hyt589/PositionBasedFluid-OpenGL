#include "ShaderProgram.hpp"

Shader::Shader(ShaderType type, std::string path){
    std::ifstream shaderFile;
    std::string shaderCode;

    try
    {
        shaderFile.open(path);
        shaderFile >> shaderCode;
        shaderFile.close();
    }
    catch(const std::exception& e)
    {
        LOG_ERR("failed to load shader file: " + path)
        return;
    }

    GLenum shaderType = type == ShaderType::VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
    const char * shaderSourceCode = shaderCode.c_str();
    ID = glCreateShader(shaderType);
    glShaderSource(ID, 1, &shaderSourceCode, NULL);
    glCompileShader(ID);
    int success;
    glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
    if (!success){
        LOG_ERR("Shader compilation failed")
        char info[512];
        glGetShaderInfoLog(ID, 512, NULL, info);
        LOG_ERR(info);
        return;
    }

    compilation_success = true;
    
}

Program::Program(unsigned int vShaderID, unsigned int fShaderID){
    ID = glCreateProgram();
    glAttachShader(ID, vShaderID);
    glAttachShader(ID, fShaderID);
    glLinkProgram(ID);
    int success;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success){
        char info[512];
        glGetProgramInfoLog(ID, 512, NULL, info);
        LOG_ERR("Program linking error")
        LOG_ERR(info)
        return;
    }
    linking_success = true;
}

void Program::use(){
    glUseProgram(ID);
}