#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fragTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

#define MAX_LIGHTS 200

struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};

uniform PointLight lights[MAX_LIGHTS];
uniform int numLights;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

uniform float fogDensity = 0.003f;
uniform vec4 fogColor = vec4(0.4f, 0.015f, 0.01f, 1.0f);

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float computeFog()
{
    float fragmentDistance = length(fPosEye.xyz);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
    return clamp(fogFactor, 0.0f, 1.0f);
}

float computeShadow(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0){
        return 0.0;
    }

    float closestDepth = texture(shadowMap, projCoords.xy).r;

    float currentDepth = projCoords.z;

    float bias = 0.005;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

void computeLightComponents()
{
    vec3 cameraPosEye = vec3(0.0f);
    vec3 normal = normalize(fNormal);
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
    vec3 texColor = texture(diffuseTexture, fragTexCoords).rgb;
    float specularMap = texture(specularTexture, fragTexCoords).r;

    ambient = vec3(0.0f);
    diffuse = vec3(0.0f);
    specular = vec3(0.0f);

    for (int i = 0; i < numLights; i++)
    {
        vec3 lightDirN = normalize(lights[i].position - fPosEye.xyz);
        float dist = length(lights[i].position - fPosEye.xyz);

        float att = 1.0f / (lights[i].constant + lights[i].linear * dist + lights[i].quadratic * (dist * dist));

        ambient += (att * ambientStrength * lights[i].color) * texColor;

        float diffCoeff = max(dot(normal, lightDirN), 0.0f);
        diffuse += (att * diffCoeff * lights[i].color) * texColor;

        vec3 reflection = reflect(-lightDirN, normal);
        float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);

        specular += att * specularStrength * specCoeff * specularMap * lights[i].color;
    }
}

void main()
{
    computeLightComponents();

    vec3 baseColor = vec3(0.9f, 0.35f, 0.25f);

    ambient *= baseColor;
    diffuse *= baseColor;
    specular *= baseColor;

    float shadow = computeShadow(fragPosLightSpace);

    vec3 color = min(ambient + (1.0 - shadow) * (diffuse + specular), 1.0f);

    float fogFactor = computeFog();
    fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
}