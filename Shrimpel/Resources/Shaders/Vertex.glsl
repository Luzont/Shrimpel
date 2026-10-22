#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoordinate;
//layout (location = 3) in mat4 InstanceMatrix;

out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TextureCoordinate;
	vec4 FragPosLightSpace;
} vs_out;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 LightSpaceMatrix;

void main()
{
	mat4 ModelMatrix = Model; //* InstanceMatrix;
	
	vs_out.FragPos = vec3(ModelMatrix * vec4(aPos, 1.0));
	vs_out.FragPosLightSpace = LightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
	vs_out.TextureCoordinate = aTextureCoordinate;
	vs_out.Normal = mat3(transpose(inverse(ModelMatrix))) * aNormal;

	gl_Position = Projection * View * ModelMatrix * vec4(aPos, 1.0);
}