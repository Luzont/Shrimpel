#version 460 core

layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec2 aTextureCoordinate;

//out vec2 TextureCoordinate;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
	gl_Position = Projection * View * Model * vec4(aPos, 1.0f);
	//TextureCoordinate = aTextureCoordinate;
}