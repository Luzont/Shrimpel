#pragma once
#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Renderer
{
	void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
	void ProcessInput(GLFWwindow* Window);
	unsigned int GenerateTexture(const char* FileName);
	int main();
};