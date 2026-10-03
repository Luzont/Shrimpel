#include <glad/glad.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <map>
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

float lastX = 400, lastY = 300;
bool firstMouse = true;

// Camera instance
Camera MainCamera(glm::vec3(0.0f, 0.0f, 3.0f));

struct PointLight {
	glm::vec3 Position;

	float Constant;
	float Linear;
	float Quadratic;

	glm::vec3 Ambient;
	glm::vec3 Diffuse;
	glm::vec3 Specular;
};

struct SpotLight {
	glm::vec3 Position;
	glm::vec3 Direction;
	float Cutoff;
	float OuterCutoff;

	float Constant;
	float Linear;
	float Quadratic;

	glm::vec3 Ambient;
	glm::vec3 Diffuse;
	glm::vec3 Specular;
};

void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
		return;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	MainCamera.ProcessMouseMovement(xoffset, yoffset);
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	MainCamera.ProcessMouseScroll(yoffset);
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void ProcessInput(GLFWwindow* Window)
{
	glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Exit
	if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(Window, true);
	}
}

unsigned int GenerateTexture(const char* FilePath, const int TextureID, const bool IsRGBA)
{
	stbi_set_flip_vertically_on_load(true);

	unsigned int Texture;
	glGenTextures(1, &Texture);
	glActiveTexture(GL_TEXTURE0 + TextureID);
	glBindTexture(GL_TEXTURE_2D, Texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int Width, Height, NRchannels;
	unsigned char* Data = stbi_load(FilePath, &Width, &Height, &NRchannels, 0);

	if (Data)
	{
		if (IsRGBA)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture " << FilePath << "!" << std::endl;
	}

	stbi_image_free(Data);

	return Texture;
}

unsigned int LoadCubemap(std::vector<std::string> Faces)
{
	stbi_set_flip_vertically_on_load(false);

	unsigned int TextureID;
	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, TextureID);

	int Width, Height, NumberOfChannels;
	for (unsigned int i = 0; i < Faces.size(); i++)
	{
		unsigned char* Data = stbi_load(Faces[i].c_str(), &Width, &Height, &NumberOfChannels, 0);

		if (Data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data);
			stbi_image_free(Data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << Faces[i] << std::endl;
			stbi_image_free(Data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return TextureID;
}

int main()
{
	MainCamera.MovementSpeed = 25;

	float Vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	float SkyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	float ScreenQuadVertices[] = {
		// positions   // texture coords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	float GrassVertices[] = {
		// positions          // normals           // texture coords
		-0.5f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
		 0.5f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
		 0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,

		 0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		-0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		-0.5f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f
	};

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	glm::vec3 PointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	unsigned int DepthMapFBO;
	unsigned int DepthMap;
	const unsigned int SHADOW_WIDTH = 32768;
	const unsigned int SHADOW_HEIGHT = 32768;

	unsigned int MSAABuffer;
	unsigned int MSAAColorBuffer;
	unsigned int MSAADepthBuffer;

	unsigned int IntermediateBuffer;
	unsigned int ResolvedColorBuffer;

	unsigned int SkyboxVBO;
	unsigned int ScreenQuadVBO;
	unsigned int CubeInstanceVertexVBO;
	unsigned int CubeInstanceMatrixVBO;
	unsigned int LightVBO;
	unsigned int ContainerVBO;

	unsigned int ScreenQuadVAO;
	unsigned int SkyboxVAO;
	unsigned int ContainerVAO;
	unsigned int CubeInstanceVAO;
	unsigned int LightVAO;

	unsigned int PointLightSSBO;
	unsigned int SpotLightSSBO;

	const unsigned short MaxLights = 64;
	std::vector<PointLight> PointLights;
	std::vector<SpotLight> SpotLights;

	const unsigned short MSAASamples = 5;

	// GLFW and GLAD initalization
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	unsigned short WindowHeight = 600;
	unsigned short WindowWidth = 800;

	GLFWwindow* Window = glfwCreateWindow(WindowWidth, WindowHeight, "OpenGL Window", NULL, NULL);

	if (Window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		return -1;
	}

	glfwMakeContextCurrent(Window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glViewport(0, 0, WindowWidth, WindowHeight);

	// Set callbacks
	glfwSetFramebufferSizeCallback(Window, FramebufferSizeCallback);
	glfwSetCursorPosCallback(Window, MouseCallback);
	glfwSetScrollCallback(Window, ScrollCallback);

	Shader MainShader("Resources/Shaders/Vertex.glsl", "Resources/Shaders/Fragment.glsl");
	Shader DepthMapShader("Resources/Shaders/DepthMapVertex.glsl", "Resources/Shaders/DepthMapFragment.glsl");
	Shader LightShader("Resources/Shaders/LightVertex.glsl", "Resources/Shaders/LightFragment.glsl");
	Shader ScreenTextureShader("Resources/Shaders/ScreenTextureVertex.glsl", "Resources/Shaders/ScreenTextureFragment.glsl");
	Shader SkyboxShader("Resources/Shaders/SkyboxVertex.glsl", "Resources/Shaders/SkyboxFragment.glsl");
	Shader SkyboxReflection("Resources/Shaders/SkyboxReflectionVertex.glsl", "Resources/Shaders/SkyboxReflectionFragment.glsl");

	// ======== DEPTH MAP ========
	glGenFramebuffers(1, &DepthMapFBO);

	glGenTextures(1, &DepthMap);
	glBindTexture(GL_TEXTURE_2D, DepthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float BorderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BorderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, DepthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ======== LIGHT SSBOs ========
	glGenBuffers(1, &PointLightSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, PointLightSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLight) * MaxLights, NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, PointLightSSBO);

	glGenBuffers(1, &SpotLightSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SpotLightSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SpotLight) * MaxLights, NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SpotLightSSBO);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// ======== MSAA FRAMEBUFFER SETUP (for rendering) ========
	glGenFramebuffers(1, &MSAABuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, MSAABuffer);

	// MSAA color buffer
	glGenTextures(1, &MSAAColorBuffer);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, MSAAColorBuffer);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAASamples, GL_RGB, WindowWidth, WindowHeight, GL_TRUE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, MSAAColorBuffer, 0);

	// MSAA depth/stencil buffer
	glGenRenderbuffers(1, &MSAADepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, MSAADepthBuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAASamples, GL_DEPTH24_STENCIL8, WindowWidth, WindowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, MSAADepthBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::MSAA_FRAMEBUFFER_INCOMPLETE" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ======== INTERMEDIATE FRAMEBUFFER SETUP (for resolving) ========
	glGenFramebuffers(1, &IntermediateBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, IntermediateBuffer);

	// Regular (non-MSAA) color buffer for resolved output
	glGenTextures(1, &ResolvedColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ResolvedColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WindowWidth, WindowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ResolvedColorBuffer, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::INTERMEDIATE_FRAMEBUFFER_INCOMPLETE" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ======== VBO SETUP ========
	glGenBuffers(1, &ContainerVBO);
	glBindBuffer(GL_ARRAY_BUFFER, ContainerVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &LightVBO);
	glBindBuffer(GL_ARRAY_BUFFER, LightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	// ======== SETUP LIGHT VAO ========
	glGenVertexArrays(1, &LightVAO);
	glBindVertexArray(LightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, LightVBO);

	// aPos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// ======== SETUP SCREEN QUAD VAO ========
	glGenVertexArrays(1, &ScreenQuadVAO);
	glBindVertexArray(ScreenQuadVAO);

	// Create a dedicated VBO for the screen quad
	glGenBuffers(1, &ScreenQuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, ScreenQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ScreenQuadVertices), ScreenQuadVertices, GL_STATIC_DRAW);

	// aPos
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// aTexCoord
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// ======== SETUP SKYBOX VAO ========
	glGenVertexArrays(1, &SkyboxVAO);
	glBindVertexArray(SkyboxVAO);

	// Create a dedicated VBO
	glGenBuffers(1, &SkyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, SkyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxVertices), SkyboxVertices, GL_STATIC_DRAW);

	// aPos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// ======== SETUP CONTAINER VAO ========
	glGenVertexArrays(1, &ContainerVAO);
	glBindBuffer(GL_ARRAY_BUFFER, ContainerVBO);
	glBindVertexArray(ContainerVAO);

	// aPos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// aNormal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// aTextureCoordinate
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	/* ======== SETUP INSTANCED CUBE VAO ========
	glGenVertexArrays(1, &CubeInstanceVAO);
	glBindVertexArray(CubeInstanceVAO);

	// Vertex data
	glGenBuffers(1, &CubeInstanceVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, CubeInstanceVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	// Vertex attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Instance matrices
	glGenBuffers(1, &CubeInstanceMatrixVBO);
	glBindBuffer(GL_ARRAY_BUFFER, CubeInstanceMatrixVBO);
	glBufferData(GL_ARRAY_BUFFER, 1 * sizeof(glm::mat4), (const void*)1, GL_STATIC_DRAW);

	// Instance matrix attributes
	std::size_t Vec4Size = sizeof(glm::vec4);
	for (int i = 0; i < 4; i++) {
		glEnableVertexAttribArray(3 + i);
		glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * Vec4Size));
		glVertexAttribDivisor(3 + i, 1);
	}

	// Unbind
	glBindVertexArray(0);

	// Give locations 3-6 (InstanceMatrix) an identity-matrix default
	glVertexAttrib4f(3, 1.0f, 0.0f, 0.0f, 0.0f);
	glVertexAttrib4f(4, 0.0f, 1.0f, 0.0f, 0.0f);
	glVertexAttrib4f(5, 0.0f, 0.0f, 1.0f, 0.0f);
	glVertexAttrib4f(6, 0.0f, 0.0f, 0.0f, 1.0f);
	commented for future reference
	*/

	// Generate Textures
	unsigned int ContainerDiffuse = GenerateTexture("Resources/Textures/container.png", 0, true);
	unsigned int MissingTexture = GenerateTexture("Resources/Textures/MissingTexture.png", 0, true);

	glm::vec3 SceneCenter(0.0f, 0.0f, 0.0f);
	float SceneRadius = 500.0f;

	glm::vec3 LightPosition(1.2f, 1.0f, 2.0f);
	glm::vec3 DirectionalLightDirection = glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f));
	glm::vec3 LightViewPos = SceneCenter - DirectionalLightDirection * SceneRadius;

	glm::mat4 LightModel = glm::mat4(1.0f);
	LightModel = glm::translate(LightModel, LightPosition);
	LightModel = glm::scale(LightModel, glm::vec3(1.0f));

	Model SceneIdk("Resources/Models/SceneThingy/scene_thingy_idk.fbx");

	glm::mat4 View;
	glm::mat4 Projection;

	float LightNearPlane = 0.1f, LightFarPlane = 2.0f * SceneRadius;
	glm::mat4 LightProjection = glm::ortho(-SceneRadius, SceneRadius, -SceneRadius, SceneRadius, LightNearPlane, LightFarPlane);
	glm::mat4 LightView = glm::lookAt(LightViewPos, SceneCenter, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 LightSpaceMatrix = LightProjection * LightView;

	std::vector<std::string> Faces =
	{
		"Resources/Textures/CubemapRight.png",
		"Resources/Textures/CubemapLeft.png",
		"Resources/Textures/CubemapTop.png",
		"Resources/Textures/CubemapBottom.png",
		"Resources/Textures/CubemapFront.png",
		"Resources/Textures/CubemapBack.png"
	};

	unsigned int CubemapTexture = LoadCubemap(Faces);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_MULTISAMPLE);

	glEnable(GL_CULL_FACE);

	PointLight PointLight1;
	PointLight PointLight2;
	SpotLight SpotLight1;

	PointLight1.Position = LightPosition;
	PointLight1.Ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	PointLight1.Diffuse = glm::vec3(0.978f, 0.2411f, 0.962f);
	PointLight1.Specular = glm::vec3(1.0f, 1.0f, 1.0f);
	PointLight1.Constant = 1.0f;
	PointLight1.Linear = 0.09f;
	PointLight1.Quadratic = 0.032f;

	SpotLight1.Position = MainCamera.Position;
	SpotLight1.Direction = MainCamera.Front;
	SpotLight1.Ambient = glm::vec3(0.0f, 0.0f, 0.0f);
	SpotLight1.Diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
	SpotLight1.Specular = glm::vec3(0.0f, 0.0f, 0.0f);
	SpotLight1.Constant = 1.0f;
	SpotLight1.Linear = 0.09f;
	SpotLight1.Quadratic = 0.032f;
	SpotLight1.Cutoff = glm::cos(glm::radians(12.5f));
	SpotLight1.OuterCutoff = glm::cos(glm::radians(15.0f));

	PointLights.push_back(PointLight1);
	SpotLights.push_back(SpotLight1);

	auto RenderScene = [&](Shader& shader) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ContainerDiffuse);

		glm::mat4 Model = glm::mat4(1.0f);
		shader.SetMat4("Model", Model);
		SceneIdk.Draw(shader); // PS: uses the backpack textures, why? because i'm lazy to map textures just to setup shadows
	};

	// Render loop
	while (!glfwWindowShouldClose(Window))
	{
		// Use the source missing texture in case texture isn't set
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, MissingTexture);

		// Inputs
		ProcessInput(Window);
		MainCamera.ProcessInputs(Window);

		// Get view and projection matrices from camera
		View = MainCamera.GetViewMatrix();
		Projection = glm::perspective(glm::radians(MainCamera.FOV), static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight), 0.001f, 1000.0f);

		// ======= RENDER TO DEPTH MAP ========
		DepthMapShader.Use();
		DepthMapShader.SetMat4("LightSpaceMatrix", LightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, DepthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		RenderScene(DepthMapShader);

		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// ======== DO DEPTH MAP STUFF ========
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, WindowWidth, WindowHeight);

		// ======== RENDER TO MSAA FRAMEBUFFER ========
		glBindFramebuffer(GL_FRAMEBUFFER, MSAABuffer);
		glClearColor(0.05f, 0.19f, 0.44f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		// ======== DRAW MAIN OBJECT ========
		MainShader.Use();

		MainShader.SetVec3("ViewPosition", MainCamera.Position);
		MainShader.SetFloat("material.Shininess", 32.0f);
		MainShader.SetInt("material.Diffuse", 0);
		MainShader.SetInt("material.Specular", 1);

		// Set uniforms for MainShader
		MainShader.SetMat4("View", View);
		MainShader.SetMat4("Projection", Projection);

		MainShader.SetVec3("directionalLight.Direction", DirectionalLightDirection);
		MainShader.SetVec3("directionalLight.Ambient", glm::vec3(0.1f, 0.1f, 0.1f));
		MainShader.SetVec3("directionalLight.Diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		MainShader.SetVec3("directionalLight.Specular", glm::vec3(0.5f, 0.5f, 0.5f));

		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, DepthMap);
		MainShader.SetMat4("LightSpaceMatrix", LightSpaceMatrix);
		MainShader.SetInt("ShadowMap", 10);

		// Do light stuff
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, PointLightSSBO);

		size_t PointLightSize = PointLights.size() * sizeof(PointLight);

		int BufferSize;
		glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &BufferSize);

		if ((GLsizei)PointLightSize > BufferSize)
			glBufferData(GL_SHADER_STORAGE_BUFFER, PointLightSize * 2, NULL, GL_DYNAMIC_DRAW);

		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, PointLightSize, PointLights.data());
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SpotLightSSBO);

		size_t SpotLightSize = SpotLights.size() * sizeof(SpotLight);

		glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &BufferSize);

		if ((GLsizei)SpotLightSize > BufferSize)
			glBufferData(GL_SHADER_STORAGE_BUFFER, SpotLightSize * 2, NULL, GL_DYNAMIC_DRAW);

		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, SpotLightSize, SpotLights.data());
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		MainShader.SetInt("NumberOfPointLights", PointLights.size());
		MainShader.SetInt("NumberOfSpotLights", SpotLights.size());

		RenderScene(MainShader);

		// ======== DRAW LIGHT OBJECT ========
		LightShader.Use();

		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(LightVAO);

		// Set uniforms for LightShader
		LightShader.SetMat4("View", View);
		LightShader.SetMat4("Projection", Projection);

		for (int i = 0; i < PointLights.size(); i++)
		{
			glm::mat4 LightModel = glm::translate(glm::mat4(1.0f), PointLights[i].Position);

			LightShader.SetMat4("Model", LightModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// ======== DRAW SKYBOX ========
		glDepthMask(GL_FALSE);
		SkyboxShader.Use();

		glm::mat4 SkyboxView = glm::mat4(glm::mat3(MainCamera.GetViewMatrix()));

		SkyboxShader.SetMat4("View", SkyboxView);
		SkyboxShader.SetMat4("Projection", Projection);

		glBindVertexArray(SkyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, CubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);

		// ======== RESOLVE MSAA TO INTERMEDIATE FRAMEBUFFER ========
		glBindFramebuffer(GL_READ_FRAMEBUFFER, MSAABuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, IntermediateBuffer);
		glBlitFramebuffer(0, 0, WindowWidth, WindowHeight, 0, 0, WindowWidth, WindowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// ======== DISPLAY RESOLVED TEXTURE ON SCREEN ========
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// ======== DO POST PROCESSING ========
		ScreenTextureShader.Use();
		glBindVertexArray(ScreenQuadVAO);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, ResolvedColorBuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);

		// Check and call events and swap the buffers
		glfwSwapBuffers(Window);
		glfwPollEvents();
	}

	glDeleteBuffers(1, &LightVBO);
	glDeleteBuffers(1, &SkyboxVBO);
	glDeleteBuffers(1, &ScreenQuadVBO);
	glDeleteBuffers(1, &ContainerVBO);
	glDeleteBuffers(1, &CubeInstanceVertexVBO);
	glDeleteBuffers(1, &CubeInstanceMatrixVBO);
	glDeleteBuffers(1, &PointLightSSBO);
	glDeleteBuffers(1, &SpotLightSSBO);
	glDeleteVertexArrays(1, &LightVAO);
	glDeleteVertexArrays(1, &ContainerVAO);
	glDeleteVertexArrays(1, &CubeInstanceVAO);
	glDeleteFramebuffers(1, &DepthMapFBO);
	glDeleteFramebuffers(1, &MSAABuffer);
	glDeleteFramebuffers(1, &IntermediateBuffer);
	glDeleteRenderbuffers(1, &MSAADepthBuffer);
	glDeleteTextures(1, &CubemapTexture);
	glDeleteTextures(1, &ContainerDiffuse);
	glDeleteTextures(1, &MissingTexture);
	glDeleteTextures(1, &DepthMap);
	glDeleteTextures(1, &MSAAColorBuffer);
	glDeleteTextures(1, &ResolvedColorBuffer);

	glfwTerminate();
	return 0;
}