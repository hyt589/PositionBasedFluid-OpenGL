#include "ShaderProgram.hpp"

std::string toString(ShaderType t){
    return t == ShaderType::VERT ? "GL_VERTEX_SHADER" :
            t == ShaderType::FRAG ? "GL_FRAGMENT_SHADER" :
            t == ShaderType::GEO ? "GL_GEOMETRY_SHADER" :
            t == ShaderType::TESS_CTL ? "GL_TESS_CONTROL_SHADER" :
            t == ShaderType::TESS_EVAL ? "GL_TESS_EVALUATION_SHADER" :
                        "GL_COMPUTE_SHADER";
}

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

    GLenum shaderType = type == ShaderType::VERT ? GL_VERTEX_SHADER :
                        type == ShaderType::FRAG ? GL_FRAGMENT_SHADER :
                        type == ShaderType::GEO ? GL_GEOMETRY_SHADER :
                        type == ShaderType::TESS_CTL ? GL_TESS_CONTROL_SHADER :
                        type == ShaderType::TESS_EVAL ? GL_TESS_EVALUATION_SHADER :
                        GL_COMPUTE_SHADER;

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

Program::Program(std::vector<std::unique_ptr<Shader>> & shaders){
    ID = glCreateProgram();
    std::set<ShaderType> set;
    for(auto & shader : shaders){
        if (set.count(shader->type))
        {
            LOG_WARN("Detected multiple [" + toString(shader->type) +"], overloading with the latest")
        }
        set.insert(shader->type);
        LOG_INFO("Attaching [" + toString(shader->type) + "]")
        glAttachShader(ID, shader->ID);
    }
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

template<typename T>
void Program::setUniform(std::string &name, T value, void (*f)(unsigned int,T)){
    unsigned int loc = glGetUniformLocation(ID, name.c_str());
    if(loc == -1){
        LOG_ERR("Uniform not found: " + name)
        return;
    }
    (*f)(loc, value);
    LOG_INFO("Uniform '" + name + "' set")
}