#define STB_IMAGE_IMPLEMENTATION
#include "asset.hpp"

Mesh::Mesh(
    std::vector<Vertex> vertices,
    std::vector<unsigned int> indices,
    std::vector<Texture> textures,
    glm::vec3 position,
    glm::vec2 orientation,
    float scale
)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    this->position = position;
    this->orientation = orientation;
    this->scale = 1.f;
    init();
}

void Mesh::Draw(Program &program){
    // program.use();

    
    // name = "mat_projection";
    // program.setUniform(name, mat_perspective_projection, glUniformMatrix4fv);

    std::set<std::string> textureTypes;
    for(size_t i = 0; i < textures.size(); i++){
        GL(glActiveTexture(GL_TEXTURE0 + i));
        program.setUniform<GLint>(textures[i].type, i, glUniform1i);
        GL(glBindTexture(GL_TEXTURE_2D, textures[i].id));
    }
    GL(glActiveTexture(GL_TEXTURE0));

    GL(glBindVertexArray(VAO));
    GL(glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0));
    GL(glBindVertexArray(0));
}


Mesh::~Mesh()
{
}


Model::Model(std::string path)
{
    loadModel(path);
}

void Model::Draw(Program &program){
    glm::mat4 mat_model(1);
    mat_model = glm::rotate(mat_model, glm::radians(orientation.x), glm::vec3(0.f, 1.f, 0.f));
    mat_model = glm::rotate(mat_model, glm::radians(orientation.x), glm::vec3(1.f, 0.f, 0.f));
    mat_model = glm::translate(mat_model, position);
    program.setUniform("mat_model", mat_model, glUniformMatrix4fv);
    for(auto mesh : meshes)
    {
        mesh.Draw(program);
    }
}

Model::~Model()
{
}