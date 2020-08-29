#pragma once

#include "global.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>
#include "ShaderProgram.hpp"
// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static uint TextureFromFile(std::string filename, std::string &dir);

struct Vertex
{
    glm::vec3 postion;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct Texture
{
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh
{
private:
    unsigned int VAO, VBO, EBO;
    void init()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(Vertex),
            &vertices[0],
            GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int),
            &indices[0],
            GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));

        glBindVertexArray(0);
    };

public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    glm::vec3 position;
    glm::vec2 orientation;
    float scale;
    Mesh(
        std::vector<Vertex> vertices,
        std::vector<unsigned int> indices,
        std::vector<Texture> textures,
        glm::vec3 position = glm::vec3(0.f, 0.f, 0.f),
        glm::vec2 orientation = glm::vec2(0.f, 0.f),
        float scale = 1.f
    );
    void Draw(Program &program);
    ~Mesh();
};

class Model
{
private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded;

    void loadModel(std::string path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_ERR("Assimp error: " << importer.GetErrorString())
            return;
        }
#ifdef _WIN32
        directory = path.substr(0, path.find_last_of('\\'));
#else
        directory = path.substr(0, path.find_last_of('/'));
#endif

        processNode(scene->mRootNode, scene);
    };

    void processNode(aiNode *node, const aiScene *scene)
    {
        for (uint i = 0; i < node->mNumMeshes; i++)
        {
            auto *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (uint i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    };

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Vertex> vertices;
        std::vector<uint> indices;
        std::vector<Texture> textures;

        for (uint i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.postion = vector;
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.uv = vec;
            }
            else
            {
                vertex.uv = glm::vec2(0.f, 0.f);
            }
        }

        for (uint i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (uint j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        if (mesh->mMaterialIndex >= 0)
        {
            auto material = scene->mMaterials[mesh->mMaterialIndex];
            auto albedoMap = loadOneMaterialTexture(
                material, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, "albedoMap");
            auto normalMap = loadMaterialTextures(
                material, aiTextureType_NORMALS, "normalMap");
            auto metallicMap = loadOneMaterialTexture(
                material, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, "metallicRoughnessMap");
            auto aoMap = loadMaterialTextures(
                material, aiTextureType_AMBIENT_OCCLUSION, "aoMap");
            textures.push_back(albedoMap);
            textures.insert(textures.end(), normalMap.begin(), normalMap.end());
            textures.push_back(metallicMap);
            textures.insert(textures.end(), aoMap.begin(), aoMap.end());
        }
        return Mesh(vertices, indices, textures);
    };

    Texture loadOneMaterialTexture(aiMaterial *mat, aiTextureType type, uint index, std::string typeName)
    {
        aiString str;
        mat->GetTexture(type, index, &str);
        bool skip = false;

        for (uint i = 0; i < textures_loaded.size(); i++)
        {
            if (std::strcmp(textures_loaded[i].path.data(), str.C_Str())==0)
            {
                skip = true;
                return textures_loaded[i];
            }
        }
        Texture tex;
        tex.id = TextureFromFile(str.C_Str(), directory);
        tex.type = typeName;
        tex.path = str.C_Str();
        textures_loaded.push_back(tex);

        return tex;
    }

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {
        std::vector<Texture> textures;
        for (uint i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;

            for (uint j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str())==0)
                {
                    skip = true;
                    textures.push_back(textures_loaded[j]);
                    break;
                }
            }

            if (!skip)
            {
                Texture tex;
                tex.id = TextureFromFile(str.C_Str(), directory);
                tex.type = typeName;
                tex.path = str.C_Str();
                textures.push_back(tex);
                textures_loaded.push_back(tex);
            }
        }
        return textures;
    }

public:
    Model(std::string path);
    void Draw(Program &program);
    ~Model();
};

static uint TextureFromFile(std::string filename, std::string &dir)
{
    std::string path;
#if defined(_WIN32) || defined(WIN32)
    path = dir + "\\" + filename;
#else
    path = dir + "/" + filename;
#endif

    uint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    u_char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        LOG_INFO("stb_image: successfully loaded " << path)
    }
    else
    {
        LOG_ERR("stb_image error: failed to load image at " << path)
        stbi_image_free(data);
    }

    return textureID;
}
