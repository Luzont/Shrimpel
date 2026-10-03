#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include "Shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TextureCoordinate;
};

struct Texture {
    unsigned int TextureID;
    std::string Type;
    std::string Path;
};

class Mesh
{
public:
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;
    std::vector<Texture> Textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(Shader& shader);

private:
    unsigned int VAO, VBO, EBO;
    void SetupMesh();
};

inline Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
    this->Vertices = vertices;
    this->Indices = indices;
    this->Textures = textures;
    SetupMesh();
}

inline void Mesh::SetupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(Vertex), &Vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(unsigned int), &Indices[0], GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    // Vertex texture coordinate
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TextureCoordinate));

    glBindVertexArray(0);
}

inline void Mesh::Draw(Shader& shader)
{
    unsigned int DiffuseTextures = 1;
    unsigned int SpecularTextures = 1;

    for (unsigned int i = 0; i < Textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);

        std::string Number;
        std::string Name = Textures[i].Type;

        if (Name == "texture_diffuse")
            Number = std::to_string(DiffuseTextures++);
        else if (Name == "texture_specular")
            Number = std::to_string(SpecularTextures++);

        shader.SetInt(("material." + Name + Number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, Textures[i].TextureID);
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}