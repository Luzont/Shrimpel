#version 460 core

out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 CameraPosition;
uniform samplerCube Skybox;

void main()
{
	vec3 DirectionVector = normalize(Position - CameraPosition);
	vec3 ReflectionVector = reflect(DirectionVector, normalize(Normal));

	FragColor = vec4(texture(Skybox, ReflectionVector).rgb, 1.0);
}