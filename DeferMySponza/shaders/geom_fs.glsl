#version 330

in vec3 varyingPosition;
in vec3 varyingNormal;
in vec4 material;

out vec3 gBufferPosition;
out vec3 gBufferNormal;
out vec4 gBufferMaterial;

void main()
{
	gBufferPosition = varyingPosition;
	gBufferNormal = varyingNormal;
	gBufferMaterial = material;
}