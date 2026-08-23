#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	unsigned int ProgramID;

	// Read and build shaders
	Shader(const char* VertexShaderPath, const char* FragmentShaderPath);

	// Use and activate the Shader Program
	const void Use();

	// Utility uniform functions
	void SetBool(const std::string &Name, bool Value) const;
	void SetInt(const std::string &Name, int Value) const;
	void SetFloat(const std::string& Name, float Value) const;
	void SetVec3(const std::string& Name, glm::vec3 Value) const;
	void SetMat4(const std::string& Name, glm::mat4 Value) const;
};


Shader::Shader(const char* VertexShaderPath, const char* FragmentShaderPath)
{
	// 1. Retrive the Vertex/Fragment Shader source from filePath
	std::string VertexCode;
	std::string FragmentCode;
	std::ifstream VertexShaderFile;
	std::ifstream FragmentShaderFile;



	// 1.1 Ensure ifstream objects can throw exceptions
	VertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	FragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// 1.2 Open the files
		VertexShaderFile.open(VertexShaderPath);
		FragmentShaderFile.open(FragmentShaderPath);
		std::stringstream VertexShaderStream, FragmentShaderStream;

		// 1.3 Read file's buffer contents into streams
		VertexShaderStream << VertexShaderFile.rdbuf();
		FragmentShaderStream << FragmentShaderFile.rdbuf();

		// 1.4 Close file handlers
		VertexShaderFile.close();
		FragmentShaderFile.close();

		// 1.5 Convert stream into string
		VertexCode = VertexShaderStream.str();
		FragmentCode = FragmentShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR:SHADER::FILE_NOT_SUCCESFULLY_READ\n" << std::endl;
	}

	const char* VertexShaderCode = VertexCode.c_str();
	const char* FragmentShaderCode = FragmentCode.c_str();

	// 2. Compile shaders
	unsigned int Vertex, Fragment;
	int Success;
	char InfoLog[512];

	// 2.1 Vertex Shader
	Vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(Vertex, 1, &VertexShaderCode, NULL);
	glCompileShader(Vertex);

	// 2.2 Print Vertex compile errors (if any)
	glGetShaderiv(Vertex, GL_COMPILE_STATUS, &Success);
	if (!Success)
	{
		glGetShaderInfoLog(Vertex, 512, NULL, InfoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << InfoLog << std::endl;
	}

	// 2.3 Fragment Shader
	Fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(Fragment, 1, &FragmentShaderCode, NULL);
	glCompileShader(Fragment);

	// 2.4 Print Fragment compile errors (if any)
	glGetShaderiv(Fragment, GL_COMPILE_STATUS, &Success);
	if (!Success)
	{
		glGetShaderInfoLog(Fragment, 512, NULL, InfoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << InfoLog << std::endl;
	}

	// 3. Create the Shader Program
	ProgramID = glCreateProgram();
	glAttachShader(ProgramID, Vertex);
	glAttachShader(ProgramID, Fragment);

	// 3.1 Link the Shader Program and output errors (if any)
	glLinkProgram(ProgramID);
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Success);
	if (!Success)
	{
		glGetProgramInfoLog(ProgramID, 512, NULL, InfoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << InfoLog << std::endl;
	}

	// 4. Delete Shaders
	glDeleteShader(Vertex);
	glDeleteShader(Fragment);
}

const void Shader::Use() 
{
	glUseProgram(ProgramID);
}

void Shader::SetBool(const std::string& Name, bool Value) const
{
	glUniform1i(glGetUniformLocation(ProgramID, Name.c_str()), (int)Value);
}

void Shader::SetInt(const std::string& Name, int Value) const
{
	glUniform1i(glGetUniformLocation(ProgramID, Name.c_str()), Value);
}

void Shader::SetFloat(const std::string& Name, float Value) const
{
	glUniform1f(glGetUniformLocation(ProgramID, Name.c_str()), Value);
}

void Shader::SetVec3(const std::string& Name, glm::vec3 Value) const
{
	glUniform3f(glGetUniformLocation(ProgramID, Name.c_str()), Value.x, Value.y, Value.z);
}

void Shader::SetMat4(const std::string& Name, glm::mat4 Value) const
{
	glUniformMatrix4fv(glGetUniformLocation(ProgramID, Name.c_str()), 1, GL_FALSE, &Value[0][0]);
}