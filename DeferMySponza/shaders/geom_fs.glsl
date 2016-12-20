#version 330

layout(std140) uniform Material
{
	vec3 colour;
	float shininess;
} material;

in vec3 varyingPosition;
in vec3 varyingNormal;

out vec3 gBufferPosition;
out vec3 gBufferNormal;

void main()
{
	gBufferPosition = varyingPosition;
	gBufferNormal = varyingNormal;
}