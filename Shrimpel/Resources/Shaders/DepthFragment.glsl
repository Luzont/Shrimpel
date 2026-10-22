#version 460 core

in vec4 FragPos;

uniform vec3 LightPosition;
uniform float FarPlane;

void main()
{
    float LightDistance = length(FragPos.xyz - LightPosition);
    LightDistance = LightDistance / FarPlane;

    gl_FragDepth = LightDistance;
}