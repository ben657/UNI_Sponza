#version 330

in vec3 varyingPosition;
in vec3 varyingNormal;

out vec3 gBufferPosition;
out vec3 gBufferNormal;

void main()
{
	gBufferPosition = varyingPosition;
	gBufferNormal = varyingNormal;
}