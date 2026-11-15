#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 uLightSpaceMatrix;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

uniform float uShadowTexelSize; // world units covered by one shadow map texel

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

	// push the shadow lookup along the normal by about a texel: a fixed distance stops being enough as soon as
	// the shadow box grows and a texel covers more ground
	vec3 offsetPos = worldPos.xyz + vNormal * uShadowTexelSize;
	vFragPosLightSpace = uLightSpaceMatrix * vec4(offsetPos, 1.0);
	gl_Position = uProj * uView * worldPos;
}