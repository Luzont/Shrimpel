#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 TextureCoordinate;

uniform mat4 Projection;
uniform mat4 View;

void main()
{
	TextureCoordinate = aPos;
	vec4 SkyboxPosition = Projection * View * vec4(aPos, 1.0);
	gl_Position = SkyboxPosition.xyww;
}