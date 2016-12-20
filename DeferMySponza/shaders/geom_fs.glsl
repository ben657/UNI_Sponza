#version 330

layout(std140) uniform Material
{
	vec3 color;
	float shininess;
} material;

in vec3 varyingPosition;
in vec3 varyingNormal;

out vec3 gBufferPosition;
out vec3 gBufferNormal;
out vec4 gBufferMaterial;

void main()
{
	gBufferPosition = varyingPosition;
	gBufferNormal = varyingNormal;
	gBufferMaterial = vec4(material.color.xyz, material.shininess);
}