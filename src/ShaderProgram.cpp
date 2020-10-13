#include "ShaderProgram.hpp"
#include <sstream>

std::string toString(ShaderType t){
    return t == ShaderType::VERT ? "GL_VERTEX_SHADER" :
            t == ShaderType::FRAG ? "GL_FRAGMENT_SHADER" :
            t == ShaderType::GEO ? "GL_GEOMETRY_SHADER" :
            t == ShaderType::TESS_CTL ? "GL_TESS_CONTROL_SHADER" :
            t == ShaderType::TESS_EVAL ? "GL_TESS_EVALUATION_SHADER" :
                        "GL_COMPUTE_SHADER";
}

Shader::Shader(ShaderType type, std::string path){

    this->type = type;
    std::ifstream shaderFile(path);
    std::string shaderCode;
    try
    {
        std::ostringstream outstream;
        outstream << shaderFile.rdbuf();
        shaderCode = outstream.str();
    }
    catch(const std::exception& e)
    {
        LOG_ERR("failed to load shader file: " + path)
        return;
    }

    GLenum shaderType = type == ShaderType::VERT ? GL_VERTEX_SHADER :
                        type == ShaderType::FRAG ? GL_FRAGMENT_SHADER :
                        type == ShaderType::GEO ? GL_GEOMETRY_SHADER :
                        type == ShaderType::TESS_CTL ? GL_TESS_CONTROL_SHADER :
                        type == ShaderType::TESS_EVAL ? GL_TESS_EVALUATION_SHADER :
                        GL_COMPUTE_SHADER;

    const char * shaderSourceCode = shaderCode.c_str();
    ID = GL(glCreateShader(shaderType));
    GL(glShaderSource(ID, 1, &shaderSourceCode, NULL));
    GL(glCompileShader(ID));
    int success;
    GL(glGetShaderiv(ID, GL_COMPILE_STATUS, &success));
    if (!success){
        LOG_ERR(toString(type) + " compilation failed")
        char info[512];
        GL(glGetShaderInfoLog(ID, 512, NULL, info));
        LOG_ERR(info);
        BREAK_POINT;
        return;
    }
    LOG_INFO(toString(type) + " successfully compiled")
    compilation_success = true;
    
}

Shader::~Shader(){
    GL(glDeleteShader(ID));
}

Program::Program(std::vector<std::unique_ptr<Shader>> & shaders){
    ID = GL(glCreateProgram());
    std::set<ShaderType> set;
    for(auto & shader : shaders){
        if (set.count(shader->type))
        {
            LOG_WARN("Detected multiple [" + toString(shader->type) +"], overloading with the latest")
        }
        set.insert(shader->type);
        LOG_INFO("Attaching [" + toString(shader->type) + "]")
        GL(glAttachShader(ID, shader->ID));
    }
    GL(glLinkProgram(ID));
    int success;
    GL(glGetProgramiv(ID, GL_LINK_STATUS, &success));
    if(!success){
        char info[512];
        GL(glGetProgramInfoLog(ID, 512, NULL, info));
        LOG_ERR("Program ID=" + std::to_string(ID) + " linking error")
        LOG_ERR(info)
        BREAK_POINT;
        GL(glDeleteProgram(ID));
        return;
    }
    linking_success = true;
    LOG_INFO("Program ID=" + std::to_string(ID) + " successfully linked");
}


void Program::activate(){
    GL(glUseProgram(ID));
}

void Program::deactivate(){
    GL(glUseProgram(0));
}

Program::~Program(){
    GL(glDeleteProgram(ID));
}

void Program::with(void (*f)(Program*, std::unordered_map<std::string, std::any>), std::unordered_map<std::string, std::any> params){
    activate();
    f(this, params);
    deactivate();
}