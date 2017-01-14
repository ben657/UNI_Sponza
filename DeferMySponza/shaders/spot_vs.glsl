#version 330

uniform mat4 combinedMatrix;

in vec3 vertexPosition;

void main(void)
{
	gl_Position = combinedMatrix * vec4(vertexPosition, 1.0);
}