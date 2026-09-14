#version 430 core

struct DirectionalLight {
	vec3 direction;
	vec3 color;
	float ambientIntensity;
	float diffuseIntensity;
	float specularIntensity;
};

struct PointLight {
    vec3 position;
    float ambientIntensity;
    vec3 color;
    float diffuseIntensity;
    float specularIntensity;
    float constant;
    float linear;
    float quadratic;
};

struct Material {
    vec3 albedo;
    float ambientReflectance;
    float specularReflectance;
    float shininess;
    float alpha;
    bool transparent;
    vec3 emission;
};

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoord;
in vec4 vFragPosLightSpace;

out vec4 FragColor;

uniform vec3 uViewPos;

uniform bool uHasDirectionalLight;
uniform bool uHasPointLight;

uniform DirectionalLight uDirectionalLight;

uniform int uPointLightCount;
layout(std430, binding = 0) buffer pointLightBuffer {
    PointLight pointLights[];
};

uniform Material uMaterial;

uniform bool uHasTexture;
uniform sampler2D uDiffuseTexture;

uniform sampler2D uShadowMap;

float calculateShadow(vec4 fragPosLS, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(0.001 * (1.0 - dot(normal, lightDir)), 0.0003);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float pcfDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;

    return shadow;
}

vec3 calculateDirectionalLight(DirectionalLight light, Material mat, vec3 normal, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), mat.shininess);

    vec3 ambient = mat.ambientReflectance * light.ambientIntensity * albedo * light.color;
    vec3 diffuse = light.diffuseIntensity * diff * albedo * light.color;
    vec3 specular = mat.specularReflectance * light.specularIntensity * spec * light.color;

    float shadow = calculateShadow(vFragPosLightSpace, normal, lightDir);

    return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 calculatePointLight(PointLight light, Material mat, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), mat.shininess);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (
        light.constant +
        light.linear * distance +
        light.quadratic * distance * distance
        );

    vec3 ambient = mat.ambientReflectance * light.ambientIntensity * albedo * light.color;
    vec3 diffuse = light.diffuseIntensity * diff * albedo * light.color;
    vec3 specular = mat.specularReflectance * light.specularIntensity * spec * light.color;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

void main()
{
    vec4 texColor = vec4(1.0);
    if (uHasTexture) {
        texColor = texture(uDiffuseTexture, vTexCoord);
        if (texColor.a < 0.1)
            discard;
    }

    vec3 normal = normalize(vNormal);
    vec3 viewDir = normalize(uViewPos - vFragPos);

    vec3 albedo = uHasTexture ? texColor.rgb * uMaterial.albedo : uMaterial.albedo;
    vec3 lighting = vec3(0.0);

    if (uHasDirectionalLight)
        lighting += calculateDirectionalLight(uDirectionalLight, uMaterial, normal, viewDir, albedo);

    for (uint i = 0; i < uPointLightCount; i++) {
        PointLight pLight = pointLights[i];
        lighting += calculatePointLight(pLight, uMaterial, normal, vFragPos, viewDir, albedo);
    }    

    vec3 finalColor = lighting + uMaterial.emission;
    FragColor = vec4(finalColor, uMaterial.alpha);
}