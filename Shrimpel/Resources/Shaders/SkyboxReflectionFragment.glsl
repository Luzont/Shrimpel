#version 460 core

out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 CameraPosition;
uniform samplerCube Skybox;

float Ratio = 1.00 / 1.52;

void main()
{
	vec3 DirectionVector = normalize(Position - CameraPosition);
	vec3 ReflectionVector = refract(DirectionVector, normalize(Normal), Ratio);

	FragColor = vec4(texture(Skybox, ReflectionVector).rgb, 1.0);
}