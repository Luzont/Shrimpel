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

	// Vertex Objects
	unsigned int Framebuffer;
	unsigned int Renderbuffer;
	unsigned int TextureColorbuffer;

	unsigned int SkyboxVBO;
	unsigned int ScreenQuadVBO;
	unsigned int StaticObjectVBO;
	unsigned int DynamicObjectVBO;

	unsigned int ScreenQuadVAO;
	unsigned int SkyboxVAO;
	unsigned int ModelVAO;
	unsigned int WindowVAO;
	unsigned int LightVAO;

	std::vector<glm::vec3> VertexArrayObjectPositions;
	std::map<float, glm::vec3> DepthSortedVertexArrayObjects;

	glm::vec3 LightPosition(1.2f, 1.0f, 2.0f);

	// GLFW and GLAD initalization
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
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
	Shader LightShader("Resources/Shaders/LightVertex.glsl", "Resources/Shaders/LightFragment.glsl");
	Shader ScreenTextureShader("Resources/Shaders/ScreenTextureVertex.glsl", "Resources/Shaders/ScreenTextureFragment.glsl");
	Shader SkyboxShader("Resources/Shaders/SkyboxVertex.glsl", "Resources/Shaders/SkyboxFragment.glsl");
	Shader SkyboxReflection("Resources/Shaders/SkyboxReflectionVertex.glsl", "Resources/Shaders/SkyboxReflectionFragment.glsl");

	// ======== FRAMEBUFFER SETUP ========
	glGenFramebuffers(1, &Framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);

	// ======== TEXTURE COLORBUFFER SETUP ========
	glGenTextures(1, &TextureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, TextureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WindowWidth, WindowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureColorbuffer, 0);

	// ========= RENDERBUFFER SETUP ========
	glGenRenderbuffers(1, &Renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, Renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WindowWidth, WindowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Renderbuffer);

	// ======== VBO SETUP ========
	glGenBuffers(1, &StaticObjectVBO);
	glBindBuffer(GL_ARRAY_BUFFER, StaticObjectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	// ======== SETUP LIGHT VAO ========
	glGenVertexArrays(1, &LightVAO);
	glBindVertexArray(LightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, StaticObjectVBO);

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

	// ======== SETUP MODEL VAO ========
	glGenVertexArrays(1, &ModelVAO);
	glGenBuffers(1, &StaticObjectVBO);
	glBindBuffer(GL_ARRAY_BUFFER, StaticObjectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
	glBindVertexArray(ModelVAO);

	// aPos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// aNormal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// aTexture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Unbind
	glBindVertexArray(0);

	/* ======== SETUP GRASS TOUCHING VAO ========
	glGenVertexArrays(1, &WindowVAO);
	glGenBuffers(1, &DynamicObjectVBO);
	glBindBuffer(GL_ARRAY_BUFFER, DynamicObjectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GrassVertices), GrassVertices, GL_STATIC_DRAW);
	glBindVertexArray(WindowVAO);

	// aPos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// aNormal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// aTexture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);*/

	// Generate Textures
	unsigned int ContainerDiffuse = GenerateTexture("Resources/Textures/container.png", 0, true);
	unsigned int MissingTexture = GenerateTexture("Resources/Textures/MissingTexture.png", 0, true);
	// unsigned int ContainerSpecular = GenerateTexture("Resources/Textures/container_specular.png", 1, true);
	// unsigned int WindowTexture = GenerateTexture("Resources/Textures/window.png", 0, true);
	
	//glm::mat4 ModelWithGrassOrSomeShitLikeThatIdk = glm::mat4(1.0f);
	//ModelWithGrassOrSomeShitLikeThatIdk = glm::translate(ModelWithGrassOrSomeShitLikeThatIdk, glm::vec3(0.0f, 0.0f, 0.0f));  // Position the main object

	glm::mat4 LightModel = glm::mat4(1.0f);
	LightModel = glm::translate(LightModel, LightPosition);
	LightModel = glm::scale(LightModel, glm::vec3(0.2f));  // Make light cube smaller

	glm::mat4 View;
	glm::mat4 Projection;

	/*std::vector<glm::vec3> Windows;
	Windows.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
	Windows.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
	Windows.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
	Windows.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
	Windows.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

	for (unsigned int i = 0; i < Windows.size(); i++)
	{
		VertexArrayObjectPositions.push_back(Windows[i]);
	}*/

	//glBindVertexArray(WindowVAO);
	//glBindTexture(GL_TEXTURE_2D, WindowTexture);

	glm::mat4 Cube = glm::mat4(1.0f);

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

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER::FRAMEBUFFER_INCOMPLETE" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
		Projection = glm::perspective(glm::radians(MainCamera.FOV), static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight), 0.001f, 100000.0f);

		// Rendering
		glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);
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

		MainShader.SetVec3("directionalLight.Direction", glm::vec3(-0.2f, -1.0f, -0.3f));
		MainShader.SetVec3("directionalLight.Ambient", glm::vec3(0.5f, 0.5f, 0.5f));
		MainShader.SetVec3("directionalLight.Diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
		MainShader.SetVec3("directionalLight.Specular", glm::vec3(0.5f, 0.5f, 0.5f));
		// point light 1
		MainShader.SetVec3("pointLights[0].Position", LightPosition);
		MainShader.SetVec3("pointLights[0].Ambient", glm::vec3(0.1f, 0.1f, 0.1f));
		MainShader.SetVec3("pointLights[0].Diffuse", glm::vec3(0.978f, 0.2411f, 0.962f));
		MainShader.SetVec3("pointLights[0].Specular", glm::vec3(1.0f, 1.0f, 1.0f));
		MainShader.SetFloat("pointLights[0].Constant", 1.0f);
		MainShader.SetFloat("pointLights[0].Linear", 0.09f);
		MainShader.SetFloat("pointLights[0].Quadratic", 0.032f);
		// point light 2
		MainShader.SetVec3("pointLights[1].Position", -LightPosition);
		MainShader.SetVec3("pointLights[1].Ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		MainShader.SetVec3("pointLights[1].Diffuse", glm::vec3(0.05f, 0.87f, 0.9f));
		MainShader.SetVec3("pointLights[1].Specular", glm::vec3(1.0f, 1.0f, 1.0f));
		MainShader.SetFloat("pointLights[1].Constant", 1.0f);
		MainShader.SetFloat("pointLights[1].Linear", 0.09f);
		MainShader.SetFloat("pointLights[1].Quadratic", 0.032f);
		// spotLight
		MainShader.SetVec3("spotLights[0].Position", MainCamera.Position);
		MainShader.SetVec3("spotLights[0].Direction", MainCamera.Front);
		MainShader.SetVec3("spotLights[0].Ambient", glm::vec3(0.0f));
		MainShader.SetVec3("spotLights[0].Diffuse", glm::vec3(0.0f));
		MainShader.SetVec3("spotLights[0].Specular", glm::vec3(0.0f));
		MainShader.SetFloat("spotLights[0].Constant", 1.0f);
		MainShader.SetFloat("spotLights[0].Linear", 0.09f);
		MainShader.SetFloat("spotLights[0].Quadratic", 0.032f);
		MainShader.SetFloat("spotLights[0].Cutoff", glm::cos(glm::radians(12.5f)));
		MainShader.SetFloat("spotLights[0].OuterCutoff", glm::cos(glm::radians(15.0f)));

		/*for (unsigned int i = 0; i < VertexArrayObjectPositions.size(); i++)
		{
			float Distance = glm::length(MainCamera.Position - VertexArrayObjectPositions[i]);
			DepthSortedVertexArrayObjects[Distance] = VertexArrayObjectPositions[i];
		}

		for (std::map<float, glm::vec3>::reverse_iterator Itterator = DepthSortedVertexArrayObjects.rbegin(); Itterator != DepthSortedVertexArrayObjects.rend(); Itterator++)
		{
			ModelWithGrassOrSomeShitLikeThatIdk = glm::mat4(1.0f);
			ModelWithGrassOrSomeShitLikeThatIdk = glm::translate(ModelWithGrassOrSomeShitLikeThatIdk, Itterator->second);
			MainShader.SetMat4("Model", ModelWithGrassOrSomeShitLikeThatIdk);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}*/

		//Backpack.Draw(MainShader);

		SkyboxReflection.Use();

		SkyboxReflection.SetMat4("View", View);
		SkyboxReflection.SetMat4("Projection", Projection);
		SkyboxReflection.SetVec3("CameraPosition", MainCamera.Position);
		SkyboxShader.SetInt("Skybox", SkyboxVAO);

		// Draw cubes 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ContainerDiffuse);
		glBindVertexArray(ModelVAO);

		for (int i = 0; i < 10; i++)
		{
			Cube = glm::mat4(1.0f);
			Cube = glm::translate(Cube, cubePositions[i]);
			SkyboxReflection.SetMat4("Model", Cube);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// ======== DRAW LIGHT OBJECT ========s
		LightShader.Use();

		// Set uniforms for LightShader
		LightShader.SetMat4("Model", LightModel);
		LightShader.SetMat4("View", View);
		LightShader.SetMat4("Projection", Projection);

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

		// Bind VAO and draw
		glBindVertexArray(LightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		DepthSortedVertexArrayObjects.clear();

		// Second pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ScreenTextureShader.Use();
		glBindVertexArray(ScreenQuadVAO);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, TextureColorbuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Check and call events and swap the buffers
		glfwSwapBuffers(Window);
		glfwPollEvents();
	}

	glDeleteBuffers(1, &LightVAO);
	glDeleteBuffers(1, &ModelVAO);
	glDeleteBuffers(1, &WindowVAO);
	glDeleteBuffers(1, &DynamicObjectVBO);
	glDeleteBuffers(1, &StaticObjectVBO);
	glDeleteFramebuffers(1, &Framebuffer);
	glDeleteRenderbuffers(1, &Renderbuffer);

	glfwTerminate();
	return 0;
}