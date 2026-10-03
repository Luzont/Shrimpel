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

in vec2 TextureCoordinate;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

float NearPlane = 0.001;
float FarPlane = 100000;

uniform int NumberOfPointLights;
uniform int NumberOfSpotLights;

uniform Material material;
uniform DirectionalLight directionalLight;
uniform vec3 ViewPosition;

uniform sampler2D ShadowMap;

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

	float Bias = max(0.05 * (1.0 - dot(Normal, LightDirection)), 0.005);
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

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 Normal, vec3 ViewDirection, float Shadow)
{
	vec3 LightDirection = normalize(-light.Direction);
	float Diff = max(dot(Normal, LightDirection), 0.0);
	vec3 HalfwayDirection = normalize(LightDirection + ViewDirection);
	float Spec = pow(max(dot(Normal, HalfwayDirection), 0.0), material.Shininess);

	vec3 Ambient  = light.Ambient  * vec3(texture(material.Diffuse, TextureCoordinate));
	vec3 Diffuse  = light.Diffuse  * Diff * vec3(texture(material.Diffuse, TextureCoordinate));
	vec3 Specular = light.Specular * Spec * vec3(texture(material.Specular, TextureCoordinate));

	return Ambient + (1.0 - Shadow) * (Diffuse + Specular);
}

vec3 CalculatePointLight(PointLight light, vec3 Normal, vec3 FragmentPosition, vec3 ViewDirection)
{
	vec3 LightDirection = normalize(light.Position - FragmentPosition);
	vec3 HalfwayDirection = normalize(LightDirection + ViewDirection);
	float Diff = max(dot(Normal, LightDirection), 0.0);

	float Spec = pow(max(dot(Normal, HalfwayDirection), 0.0), material.Shininess);

	float Distance = length(light.Position - FragmentPosition);
	float Attenuation = 1.0 / (light.Constant + light.Linear * Distance + light.Quadratic * (Distance * Distance));

	vec3 Ambient = light.Ambient * vec3(texture(material.Diffuse, TextureCoordinate));
	vec3 Diffuse = light.Diffuse * Diff * vec3(texture(material.Diffuse, TextureCoordinate));
	vec3 Specular = light.Specular * Spec * vec3(texture(material.Specular, TextureCoordinate));
	Ambient *= Attenuation;
	Diffuse *= Attenuation;
	Specular *= Attenuation;
	vec3 Result = Ambient + Diffuse + Specular;
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

	vec3 Ambient = light.Ambient * vec3(texture(material.Diffuse, TextureCoordinate));
	vec3 Diffuse = light.Diffuse * Diff * vec3(texture(material.Diffuse, TextureCoordinate));
	vec3 Specular = light.Specular * Spec * vec3(texture(material.Specular, TextureCoordinate));
	Ambient *= Attenuation;
	Diffuse *= Attenuation * Intesity;
	Specular *= Attenuation * Intesity;
	vec3 Result = Ambient + Diffuse + Specular;
	return Result;
}

void main()
{
	vec3 Norm = normalize(Normal);
	vec3 ViewDirection = normalize(ViewPosition - FragPos);

	// Shadows
	float Shadow = CalculateDirectionalShadow(directionalLight, FragPosLightSpace);

	// Directional light
	vec3 Result = CalculateDirectionalLight(directionalLight, Norm, ViewDirection, Shadow);

	// Point lights
	for (int i=0; i < NumberOfPointLights; i++)
		Result += CalculatePointLight(PointLights[i], Norm, FragPos, ViewDirection);

	// Spot lights
	for (int i=0; i < NumberOfSpotLights; i++)
		Result += CalculateSpotLight(SpotLights[i], Norm, FragPos, ViewDirection);

	FragColor = vec4(Result, 1.0);
}