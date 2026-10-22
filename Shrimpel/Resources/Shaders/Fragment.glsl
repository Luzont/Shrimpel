#version 460 core

struct Material {
	sampler2D Diffuse;
	sampler2D Specular;
	float Shininess;
};

struct DirectionalLight {
	vec3 Direction;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};

struct PointLight {
	vec3 Position;

	float Constant;
	float Linear;
	float Quadratic;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};

struct SpotLight {
	vec3 Position;
	vec3 Direction;
	float Cutoff;
	float OuterCutoff;
  
	float Constant;
	float Linear;
	float Quadratic;
  
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;       
};

layout (std430, binding = 0) buffer PointLightBlock
{
	PointLight PointLights[];
};

layout (std430, binding = 1) buffer SpotLightBlock
{
	SpotLight SpotLights[];
};

out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TextureCoordinate;
	vec4 FragPosLightSpace;
} fs_in;

uniform int NumberOfPointLights;
uniform int NumberOfSpotLights;

uniform Material material;
uniform DirectionalLight directionalLight;
uniform vec3 ViewPosition;

uniform float PointLightFarPlane;

uniform sampler2D ShadowMap;
uniform samplerCube ShadowCubeMap;

vec3 Color = vec3(texture(material.Diffuse, fs_in.TextureCoordinate));
float NearPlane = 0.001;
float FarPlane = 100;

vec3 SampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

float LinearizeDepth(float Depth)
{
	float z = Depth * 2.0 - 1.0;
	return (2.0 * NearPlane * FarPlane) / (FarPlane + NearPlane - z * (FarPlane - NearPlane));
}

float CalculateDirectionalShadow(DirectionalLight light, vec4 fragPosLightSpace)
{
	vec3 LightDirection = normalize(-light.Direction);
	vec3 ProjectionCoordinate = fragPosLightSpace.xyz / fragPosLightSpace.w;
	ProjectionCoordinate = ProjectionCoordinate * 0.5 + 0.5;

	if (ProjectionCoordinate.z > 1.0)
		return 0.0;

	float Bias = max(0.05 * (1.0 - dot(fs_in.Normal, LightDirection)), 0.005);
	float ClosestDepth = texture(ShadowMap, ProjectionCoordinate.xy).r;
	float CurrentDepth = ProjectionCoordinate.z;
	float Shadow = 0;
	vec2 TexelSize = 1.0 / textureSize(ShadowMap, 0);

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float PCFDepth = texture(ShadowMap, ProjectionCoordinate.xy + vec2(x, y) * TexelSize).r;
			Shadow += CurrentDepth - Bias > PCFDepth ? 1.0 : 0.0;
		}
	}

	Shadow /= 9.0;

	return Shadow;
}

float CalculatePointShadow(PointLight light, vec3 fragPos)
{
	vec3 FragToLight = fragPos - light.Position;
	float CurrentDepth = length(FragToLight);
    float Shadow = 0.0;
	float Bias = 0.15;
	int Samples = 40;
	float ViewDistance = length(ViewPosition - fragPos);
	float DiskRadius = 0.05;
	
	for (int i = 0; i < Samples; ++i)
	{
		float ClosestDepth = texture(ShadowCubeMap, FragToLight + SampleOffsetDirections[i] * DiskRadius).r * FarPlane;

		if (CurrentDepth - Bias > ClosestDepth)
			Shadow += 1.0;
	}

	Shadow /= float(Samples);

	return Shadow;
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 Normal, vec3 ViewDirection, float Shadow)
{
	vec3 LightDirection = normalize(-light.Direction);
	float Diff = max(dot(Normal, LightDirection), 0.0);
	vec3 HalfwayDirection = normalize(LightDirection + ViewDirection);
	float Spec = pow(max(dot(Normal, HalfwayDirection), 0.0), material.Shininess);

	vec3 Ambient = light.Ambient;
	vec3 Diffuse = light.Diffuse * Diff;
	vec3 Specular = light.Specular * Spec;

	vec3 Result = (Ambient + (1.0 - Shadow) * (Diffuse + Specular)) * Color;
	return Result;
}

vec3 CalculatePointLight(PointLight light, vec3 Normal, vec3 FragmentPosition, vec3 ViewDirection, float Shadow)
{
	vec3 LightDirection = normalize(light.Position - FragmentPosition);
	vec3 HalfwayDirection = normalize(LightDirection + ViewDirection);
	float Diff = max(dot(Normal, LightDirection), 0.0);

	float Spec = pow(max(dot(Normal, HalfwayDirection), 0.0), material.Shininess);

	float Distance = length(light.Position - FragmentPosition);
	float Attenuation = 1.0 / (light.Constant + light.Linear * Distance + light.Quadratic * (Distance * Distance));

	vec3 Ambient = light.Ambient;
	vec3 Diffuse = light.Diffuse * Diff;
	vec3 Specular = light.Specular * Spec;
	Ambient *= Attenuation;
	Diffuse *= Attenuation;
	Specular *= Attenuation;

	vec3 Result = (Ambient + (1.0 - Shadow) * (Diffuse + Specular)) * Color;

	return Result;
}

vec3 CalculateSpotLight(SpotLight light, vec3 Normal, vec3 FragmentPosition, vec3 ViewDirection)
{
	vec3 LightDirection = normalize(light.Position - FragmentPosition);
	vec3 HalfwayDirection = normalize(LightDirection + ViewDirection);
	float Diff = max(dot(Normal, LightDirection), 0.0);

	float Spec = pow(max(dot(Normal, HalfwayDirection), 0.0), material.Shininess);

	float Distance = length(light.Position - FragmentPosition);
	float Attenuation = 1.0 / (light.Constant + light.Linear * Distance + light.Quadratic * (Distance * Distance));

	float Theta = dot(LightDirection, normalize(-light.Direction));
	float Epsilon = light.Cutoff - light.OuterCutoff;
	float Intesity = clamp((Theta - light.OuterCutoff) / Epsilon, 0.0, 1.0);

	vec3 Ambient = light.Ambient;
	vec3 Diffuse = light.Diffuse * Diff;
	vec3 Specular = light.Specular * Spec;
	Ambient *= Attenuation;
	Diffuse *= Attenuation * Intesity;
	Specular *= Attenuation * Intesity;
	vec3 Result = (Ambient + Diffuse + Specular) * Color;
	return Result;
}

void main()
{
	Color = vec3(texture(material.Diffuse, fs_in.TextureCoordinate));
	vec3 Norm = normalize(fs_in.Normal);
	vec3 ViewDirection = normalize(ViewPosition - fs_in.FragPos);
	vec3 Result = vec3(0.0, 0.0, 0.0);

	// Directional light
	//float DirectionalShadow = CalculateDirectionalShadow(directionalLight, fs_in.FragPosLightSpace);
	//Result += CalculateDirectionalLight(directionalLight, Norm, ViewDirection, DirectionalShadow);

	// Point lights
	for (int i = 0; i < NumberOfPointLights; i++)
	{
		float PointShadow = CalculatePointShadow(PointLights[i], fs_in.FragPos);
		Result += CalculatePointLight(PointLights[i], Norm, fs_in.FragPos, ViewDirection, PointShadow);
	}

	// Spot lights
	//for (int i=0; i < NumberOfSpotLights; i++)
	//	Result += CalculateSpotLight(SpotLights[i], Norm, fs_in.FragPos, ViewDirection);
	
	FragColor = vec4(Result, 1.0);
}