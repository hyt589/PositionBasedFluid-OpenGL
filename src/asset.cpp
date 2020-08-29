#define STB_IMAGE_IMPLEMENTATION
#include "asset.hpp"

Mesh::Mesh(
    std::vector<Vertex> vertices,
    std::vector<unsigned int> indices,
    std::vector<Texture> textures
)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    init();
}

void Mesh::Draw(Program &program){
    program.use();
    std::set<std::string> textureTypes;
    for(size_t i = 0; i < textures.size(); i++){
        glActiveTexture(GL_TEXTURE0 + i);
        program.setUniform<GLint>(textures[i].type, i, glUniform1i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


Mesh::~Mesh()
{
}


Model::Model(std::string path)
{
    loadModel(path);
}

void Model::Draw(Program &program){
    for(auto mesh : meshes)
    {
        mesh.Draw(program);
    }
}

Model::~Model()
{
}