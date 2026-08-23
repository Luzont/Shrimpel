#version 460 core
#define NumberOfPointLights 1
#define NumberOfSpotLights 1

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

out vec4 FragColor;

in vec2 TextureCoordinate;
in vec3 Normal;
in vec3 FragPos;

float NearPlane = 0.001;
float FarPlane = 100000;

uniform sampler2D Texture1;
uniform Material material;
uniform DirectionalLight directionalLight; 
uniform PointLight pointLights[NumberOfPointLights];
uniform SpotLight spotLights[NumberOfSpotLights];
uniform vec3 ViewPosition;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 Normal, vec3 ViewDirection);
vec3 CalculatePointLight(PointLight light, vec3 Normal, vec3 FragmentPosition, vec3 ViewDirection);
vec3 CalculateSpotLight(SpotLight light, vec3 Normal, vec3 FragmentPosition, vec3 ViewDirection);

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 Normal, vec3 ViewDirection)
{
    vec3 LightDirection = normalize(-light.Direction);
    float Diff = max(dot(Normal, LightDirection), 0.0);
    vec3 ReflectionDirection = reflect(-LightDirection, Normal);
    float Spec = pow(max(dot(ViewDirection, ReflectionDirection), 0.0), material.Shininess);

    vec3 Ambient = light.Ambient * vec3(texture(material.Diffuse, TextureCoordinate));
    vec3 Diffuse = light.Diffuse * Diff * vec3(texture(material.Diffuse, TextureCoordinate));
    vec3 Specular = light.Specular * Spec * vec3(texture(material.Specular, TextureCoordinate));
    vec3 Result = Ambient + Diffuse + Specular;
    return Result;
}

vec3 CalculatePointLight(PointLight light, vec3 Normal, vec3 FragmentPosition, vec3 ViewDirection)
{
    vec3 LightDirection = normalize(light.Position - FragmentPosition);
    float Diff = max(dot(Normal, LightDirection), 0.0);

    vec3 ReflectionDirection = reflect(-LightDirection, Normal);
    float Spec = pow(max(dot(ViewDirection, ReflectionDirection), 0.0), material.Shininess);

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
    float Diff = max(dot(Normal, LightDirection), 0.0);

    vec3 ReflectionDirection = reflect(-LightDirection, Normal);
    float Spec = pow(max(dot(ViewDirection, ReflectionDirection), 0.0), material.Shininess);

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

float LinearizeDepth(float Depth)
{
    float z = Depth * 2.0 - 1.0;
    return (2.0 * NearPlane * FarPlane) / (FarPlane + NearPlane - z * (FarPlane - NearPlane));
}

void main()
{
    vec4 TextureColor = texture(Texture1, TextureCoordinate);

    vec3 Norm = normalize(Normal);
    vec3 ViewDirection = normalize(ViewPosition - FragPos);

    // Directional light
    vec3 Result = CalculateDirectionalLight(directionalLight, Norm, ViewDirection);

    // Point lights
    for (int i=0; i < NumberOfPointLights; i++)
        Result += CalculatePointLight(pointLights[i], Norm, FragPos, ViewDirection);

    // Spot lights
    for (int i=0; i< NumberOfSpotLights; i++)
        Result += CalculateSpotLight(spotLights[i], Norm, FragPos, ViewDirection);

    FragColor = vec4(Result, 1.0) * TextureColor;
}