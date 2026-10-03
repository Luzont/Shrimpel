#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include "Shader.h"
#include "Mesh.h"

class Model
{
public:
	Model(const char* FilePath)
	{
		LoadModel(FilePath);
	}
	void Draw(Shader& shader);

private:
	unsigned int GetTextureFromFile(const char* FilePath, const std::string& Directory);
	std::vector<Mesh> Meshes;
	std::vector<Texture> SharedTextures; // Albedo/Normal/Metallic/Roughness/AO — loaded once, shared by every mesh
	std::string Directory;

	void LoadPBRTextures();
	void LoadModel(std::string FilePath);
	void ProcessNode(aiNode* Node, const aiScene* Scene, const glm::mat4& ParentTransform);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* Scene, glm::mat4 Transform);
};

glm::mat4 aiMatrixToGLMMatrix(const aiMatrix4x4& MatrixToConvert)
{
	glm::mat4 GlmMatrix;
	GlmMatrix[0][0] = MatrixToConvert.a1; GlmMatrix[1][0] = MatrixToConvert.a2;
	GlmMatrix[2][0] = MatrixToConvert.a3; GlmMatrix[3][0] = MatrixToConvert.a4;
	GlmMatrix[0][1] = MatrixToConvert.b1; GlmMatrix[1][1] = MatrixToConvert.b2;
	GlmMatrix[2][1] = MatrixToConvert.b3; GlmMatrix[3][1] = MatrixToConvert.b4;
	GlmMatrix[0][2] = MatrixToConvert.c1; GlmMatrix[1][2] = MatrixToConvert.c2;
	GlmMatrix[2][2] = MatrixToConvert.c3; GlmMatrix[3][2] = MatrixToConvert.c4;
	GlmMatrix[0][3] = MatrixToConvert.d1; GlmMatrix[1][3] = MatrixToConvert.d2;
	GlmMatrix[2][3] = MatrixToConvert.d3; GlmMatrix[3][3] = MatrixToConvert.d4;
	return GlmMatrix;
}

void Model::Draw(Shader& shader)
{
	for (unsigned int i = 0; i < Meshes.size(); i++)
		Meshes[i].Draw(shader);
}

void Model::LoadModel(std::string FilePath)
{
	Assimp::Importer Importer;
	const aiScene* Scene = Importer.ReadFile(FilePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GlobalScale);

	if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << Importer.GetErrorString() << std::endl;
		return;
	}

	Directory = FilePath.substr(0, FilePath.find_last_of('/'));

	LoadPBRTextures(); // load Albedo/Normal/Metallic/Roughness/AO ONCE
	ProcessNode(Scene->mRootNode, Scene, glm::mat4(1.0f));
}

void Model::LoadPBRTextures()
{
	Texture Albedo, Normal, Metallic, Roughness, AO;

	Albedo.TextureID = GetTextureFromFile("Albedo.jpg", Directory);
	Albedo.Type = "texture_diffuse";
	SharedTextures.push_back(Albedo);

	Normal.TextureID = GetTextureFromFile("Normal.png", Directory);
	Normal.Type = "texture_normal";
	SharedTextures.push_back(Normal);

	Metallic.TextureID = GetTextureFromFile("Metallic.jpg", Directory);
	Metallic.Type = "texture_metallic";
	SharedTextures.push_back(Metallic);

	Roughness.TextureID = GetTextureFromFile("Rough.jpg", Directory);
	Roughness.Type = "texture_roughness";
	SharedTextures.push_back(Roughness);

	AO.TextureID = GetTextureFromFile("AO.jpg", Directory);
	AO.Type = "texture_ao";
	SharedTextures.push_back(AO);
}

void Model::ProcessNode(aiNode* Node, const aiScene* Scene, const glm::mat4& ParentTransform)
{
	glm::mat4 NodeTransform = aiMatrixToGLMMatrix(Node->mTransformation);
	glm::mat4 GlobalTransform = ParentTransform * NodeTransform;

	for (unsigned int i = 0; i < Node->mNumMeshes; i++)
	{
		aiMesh* mesh = Scene->mMeshes[Node->mMeshes[i]];
		Meshes.push_back(ProcessMesh(mesh, Scene, GlobalTransform));
	}

	for (unsigned int i = 0; i < Node->mNumChildren; i++)
		ProcessNode(Node->mChildren[i], Scene, GlobalTransform);
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* Scene, glm::mat4 Transform)
{
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;

	glm::mat3 NormalMatrix = glm::transpose(glm::inverse(glm::mat3(Transform)));

	// Vertex positions, normals and texture coordinates
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 Vector;

		Vector.x = mesh->mVertices[i].x;
		Vector.y = mesh->mVertices[i].y;
		Vector.z = mesh->mVertices[i].z;
		vertex.Position = glm::vec3(Transform * glm::vec4(Vector, 1.0f));

		Vector.x = mesh->mNormals[i].x;
		Vector.y = mesh->mNormals[i].y;
		Vector.z = mesh->mNormals[i].z;
		vertex.Normal = glm::normalize(NormalMatrix * Vector);

		if (mesh->mTextureCoords[0])
		{
			glm::vec2 TextureCoordinate;
			TextureCoordinate.x = mesh->mTextureCoords[0][i].x;
			TextureCoordinate.y = mesh->mTextureCoords[0][i].y;
			vertex.TextureCoordinate = TextureCoordinate;
		}
		else
			vertex.TextureCoordinate = glm::vec2(0.0f, 0.0f);

		Vertices.push_back(vertex);
	}

	// Indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace Face = mesh->mFaces[i];

		for (unsigned int j = 0; j < Face.mNumIndices; j++)
			Indices.push_back(Face.mIndices[j]);
	}

	// Textures are shared across the whole model — no per-mesh loading
	return Mesh(Vertices, Indices, SharedTextures);
}

unsigned int Model::GetTextureFromFile(const char* FilePath, const std::string& Directory)
{
	std::cout << "Getting texture from file..." << std::endl;
	std::string FileName = std::string(FilePath);
	FileName = Directory + '/' + FileName;

	unsigned int TextureID;
	glGenTextures(1, &TextureID);

	int Width, Height, NumberOfComponents;
	unsigned char* Data = stbi_load(FileName.c_str(), &Width, &Height, &NumberOfComponents, 0);

	if (Data)
	{
		GLenum Format;

		if (NumberOfComponents == 1)
			Format = GL_RED;
		else if (NumberOfComponents == 2)
			Format = GL_RG;
		else if (NumberOfComponents == 3)
			Format = GL_RGB;
		else if (NumberOfComponents == 4)
			Format = GL_RGBA;
		else
		{
			Format = GL_RGB;
			std::cout << "curl up in a ball and cry" << std::endl;
		}

		std::cout << "Number of components: " << NumberOfComponents << std::endl;

		glBindTexture(GL_TEXTURE_2D, TextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, Format, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(Data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << FilePath << std::endl;
		stbi_image_free(Data);
	}

	return TextureID;
}