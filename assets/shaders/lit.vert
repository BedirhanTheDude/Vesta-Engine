#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 uLightSpaceMatrix;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vFragPos;
out vec3 vNormal;
out vec2 vTexCoord;
out vec4 vFragPosLightSpace;

void main() {
	vec4 worldPos = uModel * vec4(aPos, 1.0);
	vFragPos = worldPos.xyz;

	mat3 normalMatrix = mat3(transpose(inverse(uModel)));
	vNormal = normalize(normalMatrix * aNormal);

	vTexCoord = aTexCoord;

	float normalOffsetScale = 0.02;
	vec3 offsetPos = worldPos.xyz + vNormal * normalOffsetScale;
	vFragPosLightSpace = uLightSpaceMatrix * vec4(offsetPos, 1.0);
	gl_Position = uProj * uView * worldPos;
}