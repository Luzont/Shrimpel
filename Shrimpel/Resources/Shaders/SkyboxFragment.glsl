#version 460 core

out vec4 FragColor;

in vec3 TextureCoordinate;

uniform samplerCube Skybox;

void main()
{
	FragColor = texture(Skybox, TextureCoordinate);
}