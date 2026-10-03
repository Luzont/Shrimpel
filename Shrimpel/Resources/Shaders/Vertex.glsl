#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoordinate;
//layout (location = 3) in mat4 InstanceMatrix;

out vec2 TextureCoordinate;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 LightSpaceMatrix;

void main()
{
	mat4 ModelMatrix = Model; //* InstanceMatrix;
	gl_Position = Projection * View * ModelMatrix * vec4(aPos, 1.0f);
	TextureCoordinate = aTextureCoordinate;
	Normal = mat3(transpose(inverse(ModelMatrix))) * aNormal;
	FragPos = vec3(ModelMatrix * vec4(aPos, 1.0f));
	FragPosLightSpace = LightSpaceMatrix * vec4(FragPos, 1.0);
}