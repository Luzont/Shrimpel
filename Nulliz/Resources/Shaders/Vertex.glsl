#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoordinate;

out vec2 TextureCoordinate;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
    gl_Position = Projection * View * Model * vec4(aPos, 1.0f);
    TextureCoordinate = aTextureCoordinate;
    Normal = mat3(transpose(inverse(Model))) * aNormal;
    FragPos = vec3(Model * vec4(aPos, 1.0f));
}